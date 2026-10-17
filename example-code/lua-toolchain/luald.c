/* Static x86-64 ELF linker with Lua memory maps; source chapter 60.
   C11 + Lua 5.4, POSIX chmod. No external linker is invoked. */
#include "common.h"
#include <sys/stat.h>

typedef struct {
    const char *name; uint64_t off,size,flags,align,addr,within;
    unsigned type,link,info,entsize; int region,placed; Bytes touched;
} InputSection;
typedef struct {
    const char *name; uint64_t value,size; unsigned bind,type,sec;
} InputSymbol;
typedef struct {
    const char *path; Bytes file; InputSection sec[NSECT]; unsigned nsec,symsec,nsym;
    InputSymbol sym[NSYM];
} Object;
typedef struct {
    char name[64]; uint64_t base,length,used,file_size,file_offset; unsigned flags;
    int nobits; Bytes data;
} Region;
typedef struct { char section[64],region[64]; uint64_t align; } Rule;
static Object *objects[NOBJ]; static unsigned nobj;
static Region regions[16]; static unsigned nregion;
static Rule rules[NSECT]; static unsigned nrule;
static char entry_name[256];

static const char *string_at(const Object *o,unsigned secno,uint64_t off)
{
    if (secno>=o->nsec || o->sec[secno].type!=3) fail("invalid string table");
    const InputSection *s=&o->sec[secno];
    if (off>=s->size) fail("string offset out of bounds");
    const char *p=(const char *)o->file.p+s->off+off;
    if (!memchr(p,0,(size_t)(s->size-off))) fail("unterminated ELF string");
    return p;
}
static void load_object(const char *path)
{
    context=path;
    if (nobj==NOBJ) fail("object capacity");
    Object *o=alloc(sizeof *o); objects[nobj++]=o; o->path=path; o->file=read_file(path);
    Bytes *b=&o->file;
    if (b->n<64 || memcmp(b->p,"\177ELF",4) || get(b,4,1)!=2 || get(b,5,1)!=1 ||
        get(b,6,1)!=1 || get(b,16,2)!=1 || get(b,18,2)!=62 || get(b,20,4)!=1 ||
        get(b,52,2)!=64 || get(b,54,2)!=0 || get(b,56,2)!=0 || get(b,58,2)!=64)
        fail("expected section-based little-endian x86-64 ELF64 ET_REL");
    uint64_t sh=get(b,40,8); o->nsec=(unsigned)get(b,60,2);
    unsigned names=(unsigned)get(b,62,2);
    if (!o->nsec || o->nsec>NSECT || names>=o->nsec || sh>b->n ||
        (uint64_t)o->nsec*64>b->n-sh) fail("invalid section header table");
    if ((get(b,7,1)!=0 && get(b,7,1)!=3) || get(b,8,1) || get(b,48,4))
        fail("unsupported ELF ABI or target flags");
    for (unsigned i=0;i<64;++i) if (get(b,sh+i,1)) fail("nonzero null section header");
    for (unsigned i=0;i<o->nsec;++i) {
        uint64_t at=sh+i*64; InputSection *s=&o->sec[i]; s->region=-1;
        s->type=(unsigned)get(b,at+4,4); s->flags=get(b,at+8,8);
        s->off=get(b,at+24,8); s->size=get(b,at+32,8);
        s->link=(unsigned)get(b,at+40,4); s->info=(unsigned)get(b,at+44,4);
        s->align=get(b,at+48,8); uint64_t ent=get(b,at+56,8);
        if (ent>UINT_MAX) fail("entry size too large");
        s->entsize=(unsigned)ent;
        if (s->size>CAP) fail("section size limit");
        if (s->type!=8 && (s->off>b->n || s->size>b->n-s->off)) fail("section exceeds file");
        if (!s->align) s->align=1;
        (void)align_up(0,s->align);
        if (s->flags&2) {
            if ((s->flags & ~UINT64_C(7)) || (s->type!=1 && s->type!=8))
                fail("unsupported allocated section flags/type");
        }
        if (s->type==2) { if (o->symsec) fail("multiple symbol tables"); o->symsec=i; }
        if (s->type==9 || s->type==17) fail("REL relocations and section groups unsupported");
    }
    for (unsigned i=0;i<o->nsec;++i)
        o->sec[i].name=string_at(o,names,get(b,sh+i*64,4));
    if (!o->symsec) fail("missing symbol table");
    InputSection *st=&o->sec[o->symsec];
    if (st->entsize!=24 || st->size%24 || !st->size || st->size/24>NSYM) fail("invalid symbol table size");
    o->nsym=(unsigned)(st->size/24);
    if (st->info>o->nsym) fail("invalid local symbol boundary");
    for (unsigned i=0;i<o->nsym;++i) {
        uint64_t at=st->off+i*24; InputSymbol *s=&o->sym[i];
        s->name=string_at(o,st->link,get(b,at,4));
        unsigned info=(unsigned)get(b,at+4,1),vis=(unsigned)get(b,at+5,1);
        s->bind=info>>4; s->type=info&15; s->sec=(unsigned)get(b,at+6,2);
        s->value=get(b,at+8,8); s->size=get(b,at+16,8);
        if (s->bind>2 || s->type>4 || (vis!=0 && vis!=2)) fail("unsupported symbol binding/type/visibility");
        if ((s->bind==0)!=(i<st->info)) fail("local symbols must precede globals");
        if (s->sec==0xfff1) { if (s->value>INT64_MAX) fail("absolute symbol exceeds supported range"); }
        else if (s->sec>=o->nsec) fail("unsupported/reserved symbol section index");
        else if (s->sec && (s->value>o->sec[s->sec].size || s->size>o->sec[s->sec].size-s->value))
            fail("symbol exceeds section");
        if (!i && (s->value || s->size || s->sec || info || vis || *s->name)) fail("invalid null symbol");
        if (i && !s->sec && !s->bind) fail("undefined local symbol");
        if (s->bind && !*s->name) fail("unnamed global symbol");
    }
    for (unsigned i=1;i<o->nsec;++i) if (o->sec[i].type==4) {
        InputSection *r=&o->sec[i];
        if (r->link!=o->symsec || !r->info || r->info>=o->nsec || r->entsize!=24 || r->size%24)
            fail("invalid RELA section");
        if (r->size/24>NREL) fail("relocation capacity");
    }
}
static void text_field(lua_State *L,int idx,const char *key,char *dst,size_t cap)
{
    lua_getfield(L,idx,key); size_t n=0;
    const char *s=lua_type(L,-1)==LUA_TSTRING ? lua_tolstring(L,-1,&n) : NULL;
    if (!s || !n || n>=cap || memchr(s,0,n)) fail("map field %s must be a nonempty short string",key);
    memcpy(dst,s,n); dst[n]=0; lua_pop(L,1);
}
static void plain_table(lua_State *L,int idx)
{
    if (!lua_istable(L,idx)) fail("map value must be a table");
    if (lua_getmetatable(L,idx)) fail("map tables must not have metatables");
}
static uint64_t integer_field(lua_State *L,int idx,const char *key)
{
    lua_getfield(L,idx,key);
    if (!lua_isinteger(L,-1) || lua_tointeger(L,-1)<0) fail("map field %s must be a nonnegative Lua integer",key);
    uint64_t v=(uint64_t)lua_tointeger(L,-1); lua_pop(L,1); return v;
}
static size_t array_length(lua_State *L,int idx,size_t max)
{
    plain_table(L,idx);
    idx=lua_absindex(L,idx); size_t n=lua_rawlen(L,idx),count=0;
    if (!n || n>max) fail("map array size");
    lua_pushnil(L);
    while (lua_next(L,idx)) {
        if (!lua_isinteger(L,-2) || lua_tointeger(L,-2)<1 || (uint64_t)lua_tointeger(L,-2)>n)
            fail("map arrays require consecutive integer keys");
        ++count; lua_pop(L,1);
    }
    if (count!=n) fail("hole in map array");
    return n;
}
static void load_map(const char *path)
{
    context=path; lua_run(path,1);
    plain_table(vm,-1);
    text_field(vm,1,"entry",entry_name,sizeof entry_name);
    lua_getfield(vm,1,"regions"); nregion=(unsigned)array_length(vm,2,16);
    for (unsigned i=0;i<nregion;++i) {
        lua_rawgeti(vm,2,i+1); plain_table(vm,3);
        Region *r=&regions[i]; char flags[8];
        text_field(vm,3,"name",r->name,sizeof r->name); text_field(vm,3,"flags",flags,sizeof flags);
        if (!strcmp(flags,"r")) r->flags=4;
        else if (!strcmp(flags,"rx")) r->flags=5;
        else if (!strcmp(flags,"rw")) r->flags=6;
        else fail("region flags must be r, rx, or rw");
        r->base=integer_field(vm,3,"origin"); r->length=integer_field(vm,3,"length");
        if (r->base<4096 || r->base%4096 || !r->length || r->length%4096 ||
            r->length>CAP || r->base>UINT64_C(0x800000000000)-r->length)
            fail("region requires page-aligned low-half address and 1..16 MiB page-multiple length");
        for (unsigned j=0;j<i;++j) {
            Region *s=&regions[j];
            if (!strcmp(r->name,s->name)) fail("duplicate region name");
            if (r->base<s->base+s->length && s->base<r->base+r->length) fail("overlapping regions");
        }
        lua_pop(vm,1);
    }
    lua_pop(vm,1); lua_getfield(vm,1,"sections"); nrule=(unsigned)array_length(vm,2,NSECT);
    for (unsigned i=0;i<nrule;++i) {
        lua_rawgeti(vm,2,i+1); plain_table(vm,3);
        Rule *r=&rules[i]; text_field(vm,3,"name",r->section,sizeof r->section);
        text_field(vm,3,"region",r->region,sizeof r->region); r->align=integer_field(vm,3,"align");
        (void)align_up(0,r->align);
        unsigned k=0; while (k<nregion && strcmp(regions[k].name,r->region)) ++k;
        if (k==nregion) fail("unknown region: %s",r->region);
        for (unsigned j=0;j<i;++j) if (!strcmp(rules[j].section,r->section)) fail("duplicate section rule");
        lua_pop(vm,1);
    }
    lua_pop(vm,2);
}
static void layout(void)
{
    for (unsigned k=0;k<nrule;++k) {
        Rule *rule=&rules[k]; unsigned rid=0;
        while (strcmp(regions[rid].name,rule->region)) ++rid;
        Region *r=&regions[rid];
        for (unsigned o=0;o<nobj;++o) for (unsigned i=1;i<objects[o]->nsec;++i) {
            InputSection *s=&objects[o]->sec[i];
            if (!(s->flags&2) || strcmp(s->name,rule->section)) continue;
            context=objects[o]->path;
            if (((s->flags&1) && !(r->flags&2)) || ((s->flags&4) && !(r->flags&1)))
                fail("section permissions incompatible with region: %s",s->name);
            s->placed=1; s->region=(int)rid;
            uint64_t alignment=s->align>rule->align?s->align:rule->align;
            uint64_t at=align_up(r->base+r->used,alignment)-r->base;
            if (at>r->length || s->size>r->length-at) fail("region overflow: %s",r->name);
            s->within=at; s->addr=r->base+at;
            if (s->size) {
                if (s->type==8) r->nobits=1;
                else {
                    if (r->nobits) fail("file-backed section after NOBITS in region");
                    zero(&r->data,(size_t)(at+s->size)-r->data.n);
                    memcpy(r->data.p+at,objects[o]->file.p+s->off,(size_t)s->size);
                    r->file_size=at+s->size;
                }
            }
            if (s->size) r->used=at+s->size;
        }
    }
    for (unsigned o=0;o<nobj;++o) for (unsigned i=1;i<objects[o]->nsec;++i) {
        InputSection *s=&objects[o]->sec[i];
        if ((s->flags&2) && s->size && !s->placed) { context=objects[o]->path; fail("unmapped section: %s",s->name); }
    }
}
typedef struct { Object *o; InputSymbol *s; } Definition;
static Definition find(const char *name)
{
    Definition d={0};
    for (unsigned j=0;j<nobj;++j) for (unsigned i=1;i<objects[j]->nsym;++i) {
        InputSymbol *s=&objects[j]->sym[i];
        if (!s->bind || !s->sec || strcmp(name,s->name)) continue;
        if (d.s && d.s->bind==1 && s->bind==1) fail("duplicate strong symbol: %s",name);
        if (!d.s || (d.s->bind==2 && s->bind==1)) d=(Definition){objects[j],s};
    }
    return d;
}
static uint64_t defined_value(Object *o,InputSymbol *s)
{
    if (s->sec==0xfff1) return s->value;
    if (!s->sec || !o->sec[s->sec].placed) fail("symbol in unplaced section: %s",s->name);
    return o->sec[s->sec].addr+s->value;
}
static uint64_t resolve(Object *o,unsigned i)
{
    if (i>=o->nsym) fail("relocation symbol out of range");
    InputSymbol *s=&o->sym[i];
    if (!i) return 0;
    if (!s->bind) return defined_value(o,s);
    Definition d=find(s->name);
    if (d.s) return defined_value(d.o,d.s);
    if (s->bind==2) return 0;
    fail("undefined symbol: %s",s->name); return 0;
}
static int64_t add_checked(int64_t a,int64_t b)
{
    if ((b>0 && a>INT64_MAX-b) || (b<0 && a<INT64_MIN-b)) fail("relocation arithmetic overflow");
    return a+b;
}
static void relocate(void)
{
    for (unsigned j=0;j<nobj;++j) {
        Object *o=objects[j]; context=o->path;
        for (unsigned i=1;i<o->nsym;++i) if (o->sym[i].bind) (void)resolve(o,i);
        for (unsigned i=1;i<o->nsec;++i) if (o->sec[i].type==4) {
            InputSection *rs=&o->sec[i],*target=&o->sec[rs->info];
            if (!(target->flags&2)) continue; /* No debug-section output. */
            if (rs->size && (target->type==8 || !target->placed)) fail("relocation into non-file-backed section");
            if (!target->touched.n) zero(&target->touched,(size_t)target->size);
            for (uint64_t k=0;k<rs->size;k+=24) {
                uint64_t at=rs->off+k,off=get(&o->file,at,8),info=get(&o->file,at+8,8);
                int64_t a=signed64(get(&o->file,at+16,8)); unsigned type=(unsigned)(info&UINT32_MAX);
                unsigned width=type==1?8:4;
                if (type!=1 && type!=2 && type!=4 && type!=10 && type!=11) fail("unsupported relocation type %u",type);
                if (off>target->size || width>target->size-off) fail("relocation field out of bounds");
                for (unsigned b=0;b<width;++b) {
                    if (target->touched.p[off+b]) fail("overlapping relocation fields");
                    target->touched.p[off+b]=1;
                }
                uint64_t s=resolve(o,(unsigned)(info>>32)),p=target->addr+off,v;
                if (type==1) v=s+(uint64_t)a; /* Full-width ELF arithmetic modulo 2^64. */
                else {
                    if (s>INT64_MAX || p>INT64_MAX) fail("relocation address range");
                    int64_t base=(int64_t)s;
                    if (type==2 || type==4) base-=(int64_t)p;
                    int64_t value=add_checked(base,a);
                    if (type==10 ? (value<0 || value>UINT32_MAX) : (value<INT32_MIN || value>INT32_MAX))
                        fail("relocation overflow for %s",o->sym[info>>32].name);
                    v=(uint64_t)value;
                }
                Region *r=&regions[target->region]; put(&r->data,(size_t)(target->within+off),v,width);
            }
        }
    }
}
static void executable(const char *path)
{
    Definition d=find(entry_name); if (!d.s) fail("entry undefined: %s",entry_name);
    uint64_t entry=defined_value(d.o,d.s);
    if (d.s->sec==0xfff1) fail("absolute entry unsupported");
    InputSection *es=&d.o->sec[d.s->sec];
    if (!(es->flags&4) || d.s->value>=es->size) fail("entry must lie inside executable section");
    unsigned used=0; for (unsigned i=0;i<nregion;++i) if (regions[i].used) ++used;
    Bytes out={0}; elf_header(&out,2); zero(&out,(used+1)*56);
    put(&out,24,entry,8); put(&out,32,64,8); put(&out,54,56,2); put(&out,56,used+1,2);
    unsigned order[16];
    for (unsigned i=0;i<nregion;++i) {
        unsigned j=i;
        while (j && regions[order[j-1]].base>regions[i].base) { order[j]=order[j-1]; --j; }
        order[j]=i;
    }
    unsigned ph=0;
    for (unsigned i=0;i<nregion;++i) {
        Region *r=&regions[order[i]]; if (!r->used) continue;
        pad(&out,4096); r->file_offset=out.n; append(&out,r->data.p,r->data.n);
        size_t at=64+ph++*56;
        put(&out,at,1,4); put(&out,at+4,r->flags,4); put(&out,at+8,r->file_offset,8);
        put(&out,at+16,r->base,8); put(&out,at+24,r->base,8);
        put(&out,at+32,r->file_size,8); put(&out,at+40,r->used,8); put(&out,at+48,4096,8);
    }
    size_t stack=64+ph*56; put(&out,stack,0x6474e551,4); put(&out,stack+4,6,4); put(&out,stack+48,16,8);
    write_file(path,&out); if (chmod(path,0755)) fail("cannot mark output executable");
    printf("entry %s 0x%" PRIx64 "\n",entry_name,entry);
    for (unsigned j=0;j<nobj;++j) for (unsigned i=1;i<objects[j]->nsec;++i) {
        InputSection *s=&objects[j]->sec[i]; if (!s->placed || !s->size) continue;
        printf("%s %s 0x%" PRIx64 " 0x%" PRIx64 "\n",objects[j]->path,s->name,s->addr,s->size);
    }
}
int main(int argc,char **argv)
{
    init(); if (argc<4) fail("usage: luald map.lua output.elf input.o ...");
    load_map(argv[1]);
    for (int i=3;i<argc;++i) load_object(argv[i]);
    layout(); relocate(); executable(argv[2]); return 0;
}
