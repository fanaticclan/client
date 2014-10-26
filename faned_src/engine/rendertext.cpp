#include "engine.h"

static inline bool htcmp(const char *key, const font &f) { return !strcmp(key, f.name); }

static hashset<font> fonts;
static font *fontdef = NULL;
static int fontdeftex = 0;

font *curfont = NULL;
int curfonttex = 0;

void newfont(char *name, char *tex, int *defaultw, int *defaulth)
{
    font *f = &fonts[name];
    if(!f->name) f->name = newstring(name);
    f->texs.shrink(0);
    f->texs.add(textureload(tex));
    f->chars.shrink(0);
    f->charoffset = '!';
    f->defaultw = *defaultw;
    f->defaulth = *defaulth;
    f->scale = f->defaulth;

    fontdef = f;
    fontdeftex = 0;
}

void fontoffset(char *c)
{
    if(!fontdef) return;
    
    fontdef->charoffset = c[0];
}

void fontscale(int *scale)
{
    if(!fontdef) return;

    fontdef->scale = *scale > 0 ? *scale : fontdef->defaulth; 
}

void fonttex(char *s)
{
    if(!fontdef) return;

    Texture *t = textureload(s);
    loopv(fontdef->texs) if(fontdef->texs[i] == t) { fontdeftex = i; return; }
    fontdeftex = fontdef->texs.length();
    fontdef->texs.add(t);
}

void fontchar(int *x, int *y, int *w, int *h, int *offsetx, int *offsety, int *advance)
{
    if(!fontdef) return;

    font::charinfo &c = fontdef->chars.add();
    c.x = *x;
    c.y = *y;
    c.w = *w ? *w : fontdef->defaultw;
    c.h = *h ? *h : fontdef->defaulth;
    c.offsetx = *offsetx;
    c.offsety = *offsety;
    c.advance = *advance ? *advance : c.offsetx + c.w;
    c.tex = fontdeftex;
}

void fontskip(int *n)
{
    if(!fontdef) return;
    loopi(max(*n, 1))
    {
        font::charinfo &c = fontdef->chars.add();
        c.x = c.y = c.w = c.h = c.offsetx = c.offsety = c.advance = c.tex = 0;
    }
}

COMMANDN(font, newfont, "ssii");
COMMAND(fontoffset, "s");
COMMAND(fontscale, "i");
COMMAND(fonttex, "s");
COMMAND(fontchar, "iiiiiii");
COMMAND(fontskip, "i");

void fontalias(const char *dst, const char *src)
{
    font *s = fonts.access(src);
    if(!s) return;
    font *d = &fonts[dst];
    if(!d->name) d->name = newstring(dst);
    d->texs = s->texs;
    d->chars = s->chars;
    d->charoffset = s->charoffset;
    d->defaultw = s->defaultw;
    d->defaulth = s->defaulth;
    d->scale = s->scale;

    fontdef = d;
    fontdeftex = d->texs.length()-1;
}

COMMAND(fontalias, "ss");

bool setfont(const char *name)
{
    font *f = fonts.access(name);
    if(!f) return false;
    curfont = f;
    return true;
}
ICOMMAND(setfont, "s", (const char *name), setfont(name)); // Fanatic Edition

static vector<font *> fontstack;

void pushfont()
{
    fontstack.add(curfont);
}

bool popfont()
{
    if(fontstack.empty()) return false;
    curfont = fontstack.pop();
    return true;
}

void gettextres(int &w, int &h)
{
    if(w < MINRESW || h < MINRESH)
    {
        if(MINRESW > w*MINRESH/h)
        {
            h = h*MINRESW/w;
            w = MINRESW;
        }
        else
        {
            w = w*MINRESH/h;
            h = MINRESH;
        }
    }
}

float text_widthf(const char *str) 
{
    float width, height;
    text_boundsf(str, width, height);
    return width;
}

#define FONTTAB (4*FONTW)
#define TEXTTAB(x) ((int((x)/FONTTAB)+1.0f)*FONTTAB)

void tabify(const char *str, int *numtabs)
{
    int tw = max(*numtabs, 0)*FONTTAB-1, tabs = 0;
    for(float w = text_widthf(str); w <= tw; w = TEXTTAB(w)) ++tabs;
    int len = strlen(str);
    char *tstr = newstring(len + tabs);
    memcpy(tstr, str, len);
    memset(&tstr[len], '\t', tabs);
    tstr[len+tabs] = '\0';
    stringret(tstr);
}

COMMAND(tabify, "si");
    
void draw_textf(const char *fstr, int left, int top, ...)
{
    defvformatstring(str, top, fstr);
    draw_text(str, left, top);
}

void draw_textfa(const char *fstr, int a, int left, int top, ...)
{
    defvformatstring(str, top, fstr);
    draw_text(str, left, top, 255, 255, 255, a, -1, -1);
}    // Fanatic Edition

static float draw_char(Texture *&tex, int c, float x, float y, float scale)
{
    font::charinfo &info = curfont->chars[c-curfont->charoffset];
    if(tex != curfont->texs[info.tex])
    {
        xtraverts += varray::end();
        tex = curfont->texs[info.tex];
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }

    float x1 = x + scale*info.offsetx,
          y1 = y + scale*info.offsety,
          x2 = x + scale*(info.offsetx + info.w),
          y2 = y + scale*(info.offsety + info.h),
          tx1 = info.x / float(tex->xs),
          ty1 = info.y / float(tex->ys),
          tx2 = (info.x + info.w) / float(tex->xs),
          ty2 = (info.y + info.h) / float(tex->ys);

    varray::attrib<float>(x1, y1); varray::attrib<float>(tx1, ty1);
    varray::attrib<float>(x2, y1); varray::attrib<float>(tx2, ty1);
    varray::attrib<float>(x2, y2); varray::attrib<float>(tx2, ty2);
    varray::attrib<float>(x1, y2); varray::attrib<float>(tx1, ty2);

    return scale*info.advance;
}

static void text_color(char c, char *stack, int size, int &sp, bvec color, int a) 
{
    if(c == 's')
    {   
        c = stack[sp];
        if(sp<size-1) stack[++sp] = c;
    }
    else
    {
        xtraverts += varray::end();
        if(c == 'r') c = stack[(sp > 0) ? --sp : sp];
        else stack[sp] = c;
        switch(c)
        {
            // Start: Fanatic Edition
            case '0': color = bvec( 64, 255, 128); break; // "Sauerbraten Green";
            case '1': color = bvec( 96, 160, 255); break; // "Sauerbraten Blue";
            case '2': color = bvec(255, 192,  64); break; // "Sauerbraten Yellow";
            case '3': color = bvec(255,  64,  64); break; // "Sauerbraten Red";
            case '4': color = bvec(128, 128, 128); break; // "Sauerbraten Gray";
            case '5': color = bvec(192,  64, 192); break; // "Sauerbraten Magenta";
            case '6': color = bvec(255, 128,   0); break; // "Sauerbraten Orange";
            case '7': color = bvec(255, 255, 255); break; // "Sauerbraten White";
            case '8': color = bvec(  0,   0,   0); break; // "Black";
            case '9': color = bvec( 80, 207, 229); break; // "Tesseract Blue";
            case 'A': color = bvec(128,   0,   0); break; // "Dark Red"
            case 'B': color = bvec(255,   0,   0); break; // "Light Red"
            case 'C': color = bvec(172,  72, 172); break; // "Dark Magenta";
            case 'D': color = bvec(255,   0, 255); break; // "Light Magenta"
            case 'E': color = bvec(255,  64, 192); break; // "Pink"
            case 'F': color = bvec( 86, 164,  56); break; // "Dark Green";
            case 'G': color = bvec(  0, 255,   0); break; //  Light Green;
            case 'H': color = bvec(48,  172, 172); break; // "Dark Cyan";
            case 'I': color = bvec( 64, 255, 255); break; // "Light Cyan";
            case 'J': color = bvec( 56,  64, 172); break; // "Dark Blue";
            case 'K': color = bvec(  0, 128, 255); break; // "Light Blue"
            case 'L': // "Loop"
            {
                int t = lastmillis % 500;
                if      (t < 100) color = bvec( 64, 255, 128);
                else if (t < 200) color = bvec( 96, 160, 255);
                else if (t < 300) color = bvec(192,  64, 192);
                else if (t < 400) color = bvec(255,  64,  64);
                else if (t < 500) color = bvec(255, 128,   0);
                break;
            }
            case 'M': color = bvec(172, 172,   0); break; // "Dark Yellow";
            case 'N': color = bvec(255, 255,   0); break; // "Light Yellow";
            // case 'O':
            // case 'P':
            // case 'Q':
            case 'R': color = bvec((rand() % 255), (rand() % 255), (rand() % 255)); break; // "Random";
            // case 'S':
            case 'T': { int m = lastmillis / 250; color = bvec( 80, 207, 229); a = m % 2 ? 96 : 255; break; } // "Blinking Tesseract Blue";
            case 'U': { int m = lastmillis / 250; color = bvec(  0,   0,   0); a = m % 2 ? 96 : 255; break; } // "Blinking Black";
            case 'V': { int m = lastmillis / 250; color = bvec(128, 128, 128); a = m % 2 ? 96 : 255; break; } // "Blinking Sauerbraten Gray";
            case 'W': { int m = lastmillis / 250; color = bvec(255, 255, 255); a = m % 2 ? 96 : 255; break; } // "Blinking Sauerbraten White";
            case 'X': { int m = lastmillis / 250; color = bvec( 64, 255, 128); a = m % 2 ? 96 : 255; break; } // "Blinking Sauerbraten Green";
            case 'Y': { int m = lastmillis / 250; color = bvec( 96, 160, 255); a = m % 2 ? 96 : 255; break; } // "Blinking Sauerbraten Blue";
            case 'Z': { int m = lastmillis / 250; color = bvec(255,  64,  64); a = m % 2 ? 96 : 255; break; } // "Blinking Sauerbraten Red";
            // End: Fanatic Edition
        }
        glColor4ub(color.x, color.y, color.z, a);
    } 
}

#define TEXTSKELETON \
    float y = 0, x = 0, scale = curfont->scale/float(curfont->defaulth);\
    int i;\
    for(i = 0; str[i]; i++)\
    {\
        TEXTINDEX(i)\
        int c = uchar(str[i]);\
        if(c=='\t')      { x = TEXTTAB(x); TEXTWHITE(i) }\
        else if(c==' ')  { x += scale*curfont->defaultw; TEXTWHITE(i) }\
        else if(c=='\n') { TEXTLINE(i) x = 0; y += FONTH; }\
        else if(c=='\f') { if(str[i+1]) { i++; TEXTCOLOR(i) }}\
        else if(curfont->chars.inrange(c-curfont->charoffset))\
        {\
            float cw = scale*curfont->chars[c-curfont->charoffset].advance;\
            if(cw <= 0) continue;\
            if(maxwidth != -1)\
            {\
                int j = i;\
                float w = cw;\
                for(; str[i+1]; i++)\
                {\
                    int c = uchar(str[i+1]);\
                    if(c=='\f') { if(str[i+2]) i++; continue; }\
                    if(i-j > 16) break;\
                    if(!curfont->chars.inrange(c-curfont->charoffset)) break;\
                    float cw = scale*curfont->chars[c-curfont->charoffset].advance;\
                    if(cw <= 0 || w + cw > maxwidth) break;\
                    w += cw;\
                }\
                if(x + w > maxwidth && j!=0) { TEXTLINE(j-1) x = 0; y += FONTH; }\
                TEXTWORD\
            }\
            else\
            { TEXTCHAR(i) }\
        }\
    }

//all the chars are guaranteed to be either drawable or color commands
#define TEXTWORDSKELETON \
                for(; j <= i; j++)\
                {\
                    TEXTINDEX(j)\
                    int c = uchar(str[j]);\
                    if(c=='\f') { if(str[j+1]) { j++; TEXTCOLOR(j) }}\
                    else { float cw = scale*curfont->chars[c-curfont->charoffset].advance; TEXTCHAR(j) }\
                }

#define TEXTEND(cursor) if(cursor >= i) { do { TEXTINDEX(cursor); } while(0); }

int text_visible(const char *str, float hitx, float hity, int maxwidth)
{
    #define TEXTINDEX(idx)
    #define TEXTWHITE(idx) if(y+FONTH > hity && x >= hitx) return idx;
    #define TEXTLINE(idx) if(y+FONTH > hity) return idx;
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += cw; TEXTWHITE(idx)
    #define TEXTWORD TEXTWORDSKELETON
    TEXTSKELETON
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
    return i;
}

//inverse of text_visible
void text_posf(const char *str, int cursor, float &cx, float &cy, int maxwidth) 
{
    #define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; break; }
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx)
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += cw;
    #define TEXTWORD TEXTWORDSKELETON if(i >= cursor) break;
    cx = cy = 0;
    TEXTSKELETON
    TEXTEND(cursor)
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void text_boundsf(const char *str, float &width, float &height, int maxwidth)
{
    #define TEXTINDEX(idx)
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx) if(x > width) width = x;
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += cw;
    #define TEXTWORD x += w;
    width = 0;
    TEXTSKELETON
    height = y + FONTH;
    TEXTLINE(_)
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void draw_text(const char *str, int left, int top, int r, int g, int b, int a, int cursor, int maxwidth) 
{
    #define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; }
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx) 
    #define TEXTCOLOR(idx) if(usecolor) text_color(str[idx], colorstack, sizeof(colorstack), colorpos, color, a);
    #define TEXTCHAR(idx) draw_char(tex, c, left+x, top+y, scale); x += cw;
    #define TEXTWORD TEXTWORDSKELETON
    char colorstack[10];
    colorstack[0] = 'c'; //indicate user color
    bvec color(r, g, b);
    int colorpos = 0;
    float cx = -FONTW, cy = 0;
    bool usecolor = true;
    if(a < 0) { usecolor = false; a = -a; }
    Texture *tex = curfont->texs[0];
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glColor4ub(color.x, color.y, color.z, a);
    varray::enable();
    varray::defattrib(varray::ATTRIB_VERTEX, 2, GL_FLOAT);
    varray::defattrib(varray::ATTRIB_TEXCOORD0, 2, GL_FLOAT);
    varray::begin(GL_QUADS);
    TEXTSKELETON
    TEXTEND(cursor)
    xtraverts += varray::end();
    if(cursor >= 0 && (totalmillis/250)&1)
    {
        glColor4ub(r, g, b, a);
        if(maxwidth != -1 && cx >= maxwidth) { cx = 0; cy += FONTH; }
        draw_char(tex, '_', left+cx, top+cy, scale);
        xtraverts += varray::end();
    }
    varray::disable();
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void reloadfonts()
{
    enumerate(fonts, font, f,
        loopv(f.texs) if(!reloadtexture(*f.texs[i])) fatal("failed to reload font texture");
    );
}

