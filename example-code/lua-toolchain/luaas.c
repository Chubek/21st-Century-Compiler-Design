/* Lua-preprocessed x86-64 assembler; source chapter 56. See README.md.
   C11 + Lua 5.4. No external assembler is invoked. */
#include "common.h"

typedef struct { const char *name; unsigned type, flags; size_t size, align; Bytes bytes; } Section;
typedef struct { char *name; unsigned bind, sec; uint64_t value; int defined; } Symbol;
typedef struct { unsigned sec, sym, type; size_t offset; int64_t addend; } Reloc;
static Section sec[4] = {{".text",1,6,0,1,{0}}, {".rodata",1,2,0,1,{0}},
                         {".data",1,3,0,1,{0}}, {".bss",8,3,0,1,{0}}};
static Symbol syms[NSYM]; static unsigned nsym, cur;
static Reloc rels[NREL]; static unsigned nrel;
static Bytes source;

static int emit(lua_State *L)
{
    size_t n; const char *s=luaL_checklstring(L,1,&n);
    if (memchr(s,0,n)) return luaL_error(L,"emit contains NUL");
    append(&source,s,n); append(&source,"\n",1);
    return 0;
}
static char *trim(char *s)
{
    while (isspace((unsigned char)*s)) ++s;
    size_t n=strlen(s);
    while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
    return s;
}
static int ident(const char *s)
{
    if (!(isalpha((unsigned char)*s) || *s=='_' || *s=='.')) return 0;
    for (++s; *s; ++s) if (!(isalnum((unsigned char)*s) || *s=='_' || *s=='.')) return 0;
    return 1;
}
static unsigned symbol(const char *name)
{
    if (!ident(name)) fail("invalid symbol: %s",name);
    for (unsigned i=0;i<nsym;++i) if (!strcmp(syms[i].name,name)) return i;
    if (nsym==NSYM) fail("symbol capacity");
    size_t n=strlen(name)+1; syms[nsym].name=alloc(n);
    memcpy(syms[nsym].name,name,n); return nsym++;
}
static int64_t number(const char *s)
{
    char *end; errno=0; int64_t v=strtoll(s,&end,0);
    if (!*s || *end || errno) fail("invalid signed 64-bit integer: %s",s);
    return v;
}
typedef struct { int sym; int64_t add; } Expr;
static Expr expression(char *s)
{
    s=trim(s);
    if (*s=='-' || *s=='+' || isdigit((unsigned char)*s)) return (Expr){-1,number(s)};
    char *p=s; while (*p && *p!='+' && *p!='-') ++p;
    int64_t add=0;
    if (*p) { char sign=*p; *p++=0; add=number(trim(p));
        if (sign=='-') { if (add==INT64_MIN) fail("expression overflow"); add=-add; }
    }
    return (Expr){(int)symbol(trim(s)),add};
}
static void bytes(uint64_t v, unsigned n)
{
    if (cur==3) fail("initialized bytes in .bss");
    size_t at=zero(&sec[cur].bytes,n); put(&sec[cur].bytes,at,v,n);
    sec[cur].size=sec[cur].bytes.n;
}
static void field(Expr e, unsigned width, unsigned type)
{
    if (e.sym<0) {
        if (type==2) fail("branch/lea requires symbol expression");
        if ((width==1 && (e.add< -128 || e.add>255)) ||
            (width==4 && (e.add<INT32_MIN || e.add>UINT32_MAX))) fail("constant field overflow");
        bytes((uint64_t)e.add,width); return;
    }
    if (width==1) fail("symbolic byte unsupported");
    if (nrel==NREL) fail("relocation capacity");
    if (type==2) { if (e.add<INT64_MIN+4) fail("addend overflow"); e.add-=4; }
    rels[nrel++]=(Reloc){cur,(unsigned)e.sym,type,sec[cur].size,e.add};
    bytes(0,width);
}
static int reg(const char *s, unsigned *width)
{
    static const char *const r64[]={"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi"};
    static const char *const r32[]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
    for (int i=0;i<8;++i) {
        if (!strcmp(s,r64[i])) { *width=8; return i; }
        if (!strcmp(s,r32[i])) { *width=4; return i; }
    }
    return -1;
}
static void instruction(char *op,char *arg)
{
    if (cur!=0) fail("instructions require .text");
    if (!strcmp(op,"ret") || !strcmp(op,"syscall") || !strcmp(op,"nop") || !strcmp(op,"ud2")) {
        if (*arg) fail("unexpected operand");
        if (!strcmp(op,"ret")) bytes(0xc3,1);
        else if (!strcmp(op,"nop")) bytes(0x90,1);
        else { bytes(0x0f,1); bytes(!strcmp(op,"syscall")?0x05:0x0b,1); }
        return;
    }
    if (!strcmp(op,"call") || !strcmp(op,"jmp") || !strcmp(op,"jz") || !strcmp(op,"jnz")) {
        if (*op=='j' && op[1]!='m') { bytes(0x0f,1); bytes(op[1]=='z'?0x84:0x85,1); }
        else bytes(*op=='c'?0xe8:0xe9,1);
        field(expression(arg),4,2); return;
    }
    char *comma=strchr(arg,','); if (!comma) fail("expected two operands");
    *comma++=0; char *a=trim(arg),*b=trim(comma);
    unsigned aw=0,bw=0; int ar=reg(a,&aw),br=reg(b,&bw);
    if (ar<0) fail("expected destination register");
    if (!strcmp(op,"lea")) {
        if (aw!=8) fail("lea requires 64-bit register");
        bytes(0x48,1); bytes(0x8d,1); bytes((unsigned)(ar<<3)|5u,1);
        field(expression(b),4,2); return;
    }
    if (!strcmp(op,"mov") && br<0) {
        if (aw==8) bytes(0x48,1);
        bytes(0xb8u+(unsigned)ar,1); field(expression(b),aw,aw==8?1:10); return;
    }
    if (!strcmp(op,"mov") || !strcmp(op,"xor")) {
        if (br<0 || aw!=bw) fail("register widths must match");
        if (aw==8) bytes(0x48,1);
        bytes(!strcmp(op,"mov")?0x89:0x31,1);
        bytes(0xc0u|((unsigned)br<<3)|(unsigned)ar,1); return;
    }
    if (!strcmp(op,"add") || !strcmp(op,"sub") || !strcmp(op,"cmp")) {
        int64_t v=number(b); if (v<INT32_MIN || v>INT32_MAX) fail("signed imm32 overflow");
        if (aw==8) bytes(0x48,1);
        bytes(0x81,1); unsigned ext=!strcmp(op,"add")?0:(!strcmp(op,"sub")?5:7);
        bytes(0xc0u|(ext<<3)|(unsigned)ar,1); bytes((uint64_t)v,4); return;
    }
    fail("unsupported opcode: %s",op);
}
static void parse_line(char *s)
{
    char *comment=strchr(s,';'); if (comment) *comment=0;
    s=trim(s); if (!*s) return;
    char *colon=strchr(s,':');
    if (colon) {
        *colon=0; unsigned k=symbol(trim(s));
        if (syms[k].defined) fail("duplicate label: %s",s);
        syms[k].defined=1; syms[k].sec=cur+1; syms[k].value=sec[cur].size;
        s=trim(colon+1); if (!*s) return;
    }
    char *arg=s; while (*arg && !isspace((unsigned char)*arg)) ++arg;
    if (*arg) *arg++=0;
    arg=trim(arg);
    for (unsigned i=0;i<4;++i) if (!strcmp(s,sec[i].name)) {
        if (*arg) fail("section directive takes no operand");
        cur=i; return;
    }
    if (!strcmp(s,".global") || !strcmp(s,".extern") || !strcmp(s,".weak")) {
        unsigned k=symbol(arg),bind=!strcmp(s,".weak")?2:1;
        if (syms[k].bind && syms[k].bind!=bind) fail("conflicting binding");
        syms[k].bind=bind; return;
    }
    if (!strcmp(s,".align") || !strcmp(s,".zero")) {
        int64_t n=number(arg); if (n<0 || n>CAP) fail("invalid size/alignment");
        size_t count=(size_t)n;
        if (!strcmp(s,".align")) {
            count=(size_t)align_up(sec[cur].size,(uint64_t)n)-sec[cur].size;
            if ((size_t)n>sec[cur].align) sec[cur].align=(size_t)n;
        }
        if (count>CAP-sec[cur].size) fail("section capacity");
        if (cur!=3) zero(&sec[cur].bytes,count);
        sec[cur].size+=count; return;
    }
    unsigned w=!strcmp(s,".byte")?1:(!strcmp(s,".long")?4:(!strcmp(s,".quad")?8:0));
    if (w) { field(expression(arg),w,w==8?1:10); return; }
    instruction(s,arg);
}
static void object_write(const char *path)
{
    Bytes out={0},names={0},strings={0},symtab={0},rt[4]={{0}};
    elf_header(&out,1); stradd(&names,""); stradd(&strings,""); zero(&symtab,24);
    unsigned ids[NSYM],firstglobal=1,index=1;
    for (unsigned pass=0;pass<2;++pass) for (unsigned i=0;i<nsym;++i) {
        Symbol *s=&syms[i];
        if (!s->defined && !s->bind) fail("undeclared external: %s",s->name);
        if ((s->bind!=0)!=pass) continue;
        ids[i]=index++; if (!pass) ++firstglobal;
        size_t at=zero(&symtab,24);
        put(&symtab,at,stradd(&strings,s->name),4); put(&symtab,at+4,s->bind<<4,1);
        put(&symtab,at+6,s->sec,2); put(&symtab,at+8,s->value,8);
    }
    for (unsigned i=0;i<nrel;++i) {
        Reloc *r=&rels[i]; size_t at=zero(&rt[r->sec],24);
        put(&rt[r->sec],at,r->offset,8);
        put(&rt[r->sec],at+8,((uint64_t)ids[r->sym]<<32)|r->type,8);
        put(&rt[r->sec],at+16,(uint64_t)r->addend,8);
    }
    uint32_t sn[12]={0}; size_t off[12]={0},sz[12]={0};
    for (unsigned i=0;i<4;++i) {
        sn[i+1]=(uint32_t)stradd(&names,sec[i].name); pad(&out,sec[i].align);
        off[i+1]=out.n; sz[i+1]=sec[i].size;
        if (i!=3) append(&out,sec[i].bytes.p,sec[i].bytes.n);
    }
    const char *rn[]={".rela.text",".rela.rodata",".rela.data",".rela.bss"};
    for (unsigned i=0;i<4;++i) {
        sn[5+i]=(uint32_t)stradd(&names,rn[i]); pad(&out,8);
        off[5+i]=append(&out,rt[i].p,rt[i].n); sz[5+i]=rt[i].n;
    }
    sn[9]=(uint32_t)stradd(&names,".symtab"); pad(&out,8);
    off[9]=append(&out,symtab.p,symtab.n); sz[9]=symtab.n;
    sn[10]=(uint32_t)stradd(&names,".strtab"); off[10]=append(&out,strings.p,strings.n); sz[10]=strings.n;
    sn[11]=(uint32_t)stradd(&names,".shstrtab"); off[11]=append(&out,names.p,names.n); sz[11]=names.n;
    pad(&out,8); size_t sh=zero(&out,12*64);
    for (unsigned i=1;i<12;++i) {
        unsigned type=3,link=0,info=0; uint64_t flags=0,a=1,e=0;
        if (i<=4) { type=sec[i-1].type; flags=sec[i-1].flags; a=sec[i-1].align; }
        else if (i<=8) { type=4; link=9; info=i-4; a=8; e=24; }
        else if (i==9) { type=2; link=10; info=firstglobal; a=8; e=24; }
        shdr(&out,sh+i*64,sn[i],type,flags,off[i],sz[i],link,info,a,e);
    }
    put(&out,40,sh,8); put(&out,58,64,2); put(&out,60,12,2); put(&out,62,11,2);
    write_file(path,&out);
}
int main(int argc,char **argv)
{
    init(); int plain=argc==4 && !strcmp(argv[1],"--asm");
    if (!(argc==3 || plain)) fail("usage: luaas [--asm] input output.o");
    context=argv[plain?2:1]; const char *output=argv[plain?3:2];
    if (plain) source=read_file(context);
    else {
        vm=luaL_newstate(); if (!vm) fail("Lua allocation failure");
        luaL_openlibs(vm); lua_pushcfunction(vm,emit); lua_setglobal(vm,"emit");
        if (luaL_loadfilex(vm,context,"t") || lua_pcall(vm,0,0,0))
            fail("Lua: %s",lua_tostring(vm,-1) ? lua_tostring(vm,-1) : "non-string error");
    }
    if (source.n && memchr(source.p,0,source.n)) fail("NUL in assembly source");
    append(&source,"\0",1); char *s=(char *)source.p;
    while (*s) { char *end=strchr(s,'\n'); if (end) *end=0; ++line_no;
        parse_line(s); if (!end) break; s=end+1;
    }
    line_no=0; object_write(output); return 0;
}
