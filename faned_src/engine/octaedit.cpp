#include "engine.h"

extern int outline;

bool boxoutline = false;

void boxs(int orient, vec o, const vec &s)
{
    int d = dimension(orient), dc = dimcoord(orient);
    float f = boxoutline ? (dc>0 ? 0.2f : -0.2f) : 0;
    o[D[d]] += dc * s[D[d]] + f;

    glBegin(GL_LINE_LOOP);

    glVertex3fv(o.v); o[R[d]] += s[R[d]];
    glVertex3fv(o.v); o[C[d]] += s[C[d]];
    glVertex3fv(o.v); o[R[d]] -= s[R[d]];
    glVertex3fv(o.v);

    glEnd();
    xtraverts += 4;
}

void boxs3D(const vec &o, vec s, int g)
{
    s.mul(g);
    loopi(6)
        boxs(i, o, s);
}

void boxsgrid(int orient, vec o, vec s, int g)
{
    int d = dimension(orient), dc = dimcoord(orient);
    float ox = o[R[d]],
          oy = o[C[d]],
          xs = s[R[d]],
          ys = s[C[d]],
          f = boxoutline ? (dc>0 ? 0.2f : -0.2f) : 0;

    o[D[d]] += dc * s[D[d]]*g + f;

    glBegin(GL_LINES);
    loop(x, xs) {
        o[R[d]] += g;
        glVertex3fv(o.v);
        o[C[d]] += ys*g;
        glVertex3fv(o.v);
        o[C[d]] = oy;
    }
    loop(y, ys) {
        o[C[d]] += g;
        o[R[d]] = ox;
        glVertex3fv(o.v);
        o[R[d]] += xs*g;
        glVertex3fv(o.v);
    }
    glEnd();
    xtraverts += 2*int(xs+ys);
}

selinfo sel, lastsel, savedsel;

int orient = 0;
int gridsize = 8;
ivec cor, lastcor;
ivec cur, lastcur;

extern int entediting;
bool editmode = false;
bool havesel = false;
bool hmapsel = false;
int horient  = 0;

extern int entmoving;

VARF(dragging, 0, 0, 1,
    if(!dragging || cor[0]<0) return;
    lastcur = cur;
    lastcor = cor;
    sel.grid = gridsize;
    sel.orient = orient;
);

int moving = 0;
ICOMMAND(moving, "b", (int *n),
{
    if(*n >= 0)
    {
        if(!*n || (moving<=1 && !pointinsel(sel, cur.tovec().add(1)))) moving = 0;
        else if(!moving) moving = 1;
    }
    intret(moving);
});

VARF(gridpower, 0, 3, 12,
{
    if(dragging) return;
    gridsize = 1<<gridpower;
    if(gridsize>=worldsize) gridsize = worldsize/2;
    cancelsel();
});

VAR(passthroughsel, 0, 0, 1);
VAR(editing, 1, 0, 0);
VAR(selectcorners, 0, 0, 1);
VARF(hmapedit, 0, 0, 1, horient = sel.orient);

void forcenextundo() { lastsel.orient = -1; }

extern void hmapcancel();

void cubecancel()
{
    havesel = false;
    moving = dragging = hmapedit = passthroughsel = 0;
    forcenextundo();
    hmapcancel();
}

void cancelsel()
{
    cubecancel();
    entcancel();
}

void toggleedit(bool force)
{
    if(!force)
    {
        if(!isconnected()) return;
        if(player->state!=CS_ALIVE && player->state!=CS_DEAD && player->state!=CS_EDITING) return; // do not allow dead players to edit to avoid state confusion
        if(!game::allowedittoggle()) return;      // not in most multiplayer modes
    }
    if(!(editmode = !editmode))
    {
        player->state = player->editstate;
        player->o.z -= player->eyeheight;    // entinmap wants feet pos
        entinmap(player);                    // find spawn closest to current floating pos
    }
    else
    {
        game::resetgamestate();
        player->editstate = player->state;
        player->state = CS_EDITING;
    }
    cancelsel();
    stoppaintblendmap();
    keyrepeat(editmode);
    editing = entediting = editmode;
    extern int fullbright;
    if(fullbright) { initlights(); lightents(); }
    if(!force) game::edittoggled(editmode);
}

VARP(selectionguard, 0, 1, 1); // Fanatic Edition

bool noedit(bool view, bool msg)
{
    if(!editmode) { if(msg) conoutf(CON_ERROR, "\f9FanEd\f7::noedit: \f3#error: \f7operation only allowed in edit mode"); return true; } // Fanatic Edition
    if(view || haveselent()) return false;
    float r = 1.0f;
    vec o = sel.o.tovec(), s = sel.s.tovec();
    s.mul(float(sel.grid) / 2.0f);
    o.add(s);
    r = float(max(s.x, max(s.y, s.z)));
    bool viewable = !selectionguard ? true : (isvisiblesphere(r, o) != VFC_NOT_VISIBLE); // Fanatic Edition
    if(!viewable && msg) conoutf(CON_ERROR, "selection not in view");
    return !viewable;
}

void reorient()
{
    sel.cx = 0;
    sel.cy = 0;
    sel.cxs = sel.s[R[dimension(orient)]]*2;
    sel.cys = sel.s[C[dimension(orient)]]*2;
    sel.orient = orient;
}

void selextend()
{
    if(noedit(true)) return;
    loopi(3)
    {
        if(cur[i]<sel.o[i])
        {
            sel.s[i] += (sel.o[i]-cur[i])/sel.grid;
            sel.o[i] = cur[i];
        }
        else if(cur[i]>=sel.o[i]+sel.s[i]*sel.grid)
        {
            sel.s[i] = (cur[i]-sel.o[i])/sel.grid+1;
        }
    }
}

ICOMMAND(edittoggle, "", (), toggleedit(false));
COMMAND(entcancel, "");
COMMAND(cubecancel, "");
COMMAND(cancelsel, "");
COMMAND(reorient, "");
COMMAND(selextend, "");

ICOMMAND(selmoved, "", (), { if(noedit(true)) return; intret(sel.o != savedsel.o ? 1 : 0); });
ICOMMAND(selsave, "", (), { if(noedit(true)) return; savedsel = sel; });
ICOMMAND(selrestore, "", (), { if(noedit(true)) return; sel = savedsel; });
ICOMMAND(selswap, "", (), { if(noedit(true)) return; swap(sel, savedsel); });

///////// selection support /////////////

cube &blockcube(int x, int y, int z, const block3 &b, int rgrid) // looks up a world cube, based on coordinates mapped by the block
{
    int dim = dimension(b.orient), dc = dimcoord(b.orient);
    ivec s(dim, x*b.grid, y*b.grid, dc*(b.s[dim]-1)*b.grid);
    s.add(b.o);
    if(dc) s[dim] -= z*b.grid; else s[dim] += z*b.grid;
    return lookupcube(s.x, s.y, s.z, rgrid);
}

#define loopxy(b)        loop(y,(b).s[C[dimension((b).orient)]]) loop(x,(b).s[R[dimension((b).orient)]])
#define loopxyz(b, r, f) { loop(z,(b).s[D[dimension((b).orient)]]) loopxy((b)) { cube &c = blockcube(x,y,z,b,r); f; } }
#define loopselxyz(f)    { if(local) makeundo(); loopxyz(sel, sel.grid, f); changed(sel); }
#define selcube(x, y, z) blockcube(x, y, z, sel, sel.grid)

////////////// cursor ///////////////

int selchildcount = 0, selchildmat = -1;

ICOMMAND(havesel, "", (), intret(havesel ? selchildcount : 0));

void countselchild(cube *c, const ivec &cor, int size)
{
    ivec ss = ivec(sel.s).mul(sel.grid);
    loopoctaboxsize(cor, size, sel.o, ss)
    {
        ivec o(i, cor.x, cor.y, cor.z, size);
        if(c[i].children) countselchild(c[i].children, o, size/2);
        else 
        {
            selchildcount++;
            if(c[i].material != MAT_AIR && selchildmat != MAT_AIR)
            {
                if(selchildmat < 0) selchildmat = c[i].material;
                else if(selchildmat != c[i].material) selchildmat = MAT_AIR;
            }
        }
    }
}

void normalizelookupcube(int x, int y, int z)
{
    if(lusize>gridsize)
    {
        lu.x += (x-lu.x)/gridsize*gridsize;
        lu.y += (y-lu.y)/gridsize*gridsize;
        lu.z += (z-lu.z)/gridsize*gridsize;
    }
    else if(gridsize>lusize)
    {
        lu.x &= ~(gridsize-1);
        lu.y &= ~(gridsize-1);
        lu.z &= ~(gridsize-1);
    }
    lusize = gridsize;
}

void updateselection()
{
    sel.o.x = min(lastcur.x, cur.x);
    sel.o.y = min(lastcur.y, cur.y);
    sel.o.z = min(lastcur.z, cur.z);
    sel.s.x = abs(lastcur.x-cur.x)/sel.grid+1;
    sel.s.y = abs(lastcur.y-cur.y)/sel.grid+1;
    sel.s.z = abs(lastcur.z-cur.z)/sel.grid+1;
}

bool editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first)
{
    plane pl(d, off);
    float dist = 0.0f;
    if(!pl.rayintersect(player->o, ray, dist))
        return false;

    dest = vec(ray).mul(dist).add(player->o);
    if(first) handle = vec(dest).sub(o);
    dest.sub(handle);
    return true;
}

inline bool isheightmap(int orient, int d, bool empty, cube *c);
extern void entdrag(const vec &ray);
extern bool hoveringonent(int ent, int orient);
extern void renderentselection(const vec &o, const vec &ray, bool entmoving);
extern float rayent(const vec &o, const vec &ray, float radius, int mode, int size, int &orient, int &ent);

// Start: Fanatic Edition
HVARP(gridselectioncolor, 0, 0x323232, 0xFFFFFF);
HVARP(grid2Dselectioncolor, 0, 0xC8C8C8, 0xFFFFFF);
HVARP(grid3Dselectioncolor, 0, 0x000078, 0xFFFFFF);
HVARP(gridhoverselectioncolor, 0, 0x787878, 0xFFFFFF);
// End: Fanatic Edition

VAR(gridlookup, 0, 0, 1);
VAR(passthroughcube, 0, 1, 1);

void rendereditcursor()
{
    int d   = dimension(sel.orient),
        od  = dimension(orient),
        odc = dimcoord(orient);

    bool hidecursor = g3d_windowhit(true, false) || blendpaintmode, hovering = false;
    hmapsel = false;

    if(moving)
    {
        static vec dest, handle;
        if(editmoveplane(sel.o.tovec(), camdir, od, sel.o[D[od]]+odc*sel.grid*sel.s[D[od]], handle, dest, moving==1))
        {
            if(moving==1)
            {
                dest.add(handle);
                handle = ivec(handle).mask(~(sel.grid-1)).tovec();
                dest.sub(handle);
                moving = 2;
            }
            ivec o = ivec(dest).mask(~(sel.grid-1));
            sel.o[R[od]] = o[R[od]];
            sel.o[C[od]] = o[C[od]];
        }
    }
    else
    if(entmoving)
    {
        entdrag(camdir);
    }
    else
    {
        ivec w;
        float sdist = 0, wdist = 0, t;
        int entorient = 0, ent = -1;

        wdist = rayent(player->o, camdir, 1e16f,
                       (editmode && showmat ? RAY_EDITMAT : 0)   // select cubes first
                       | (!dragging && entediting ? RAY_ENTS : 0)
                       | RAY_SKIPFIRST
                       | (passthroughcube==1 ? RAY_PASS : 0), gridsize, entorient, ent);

        if((havesel || dragging) && !passthroughsel && !hmapedit)     // now try selecting the selection
            if(rayboxintersect(sel.o.tovec(), vec(sel.s.tovec()).mul(sel.grid), player->o, camdir, sdist, orient))
            {   // and choose the nearest of the two
                if(sdist < wdist)
                {
                    wdist = sdist;
                    ent   = -1;
                }
            }

        if((hovering = hoveringonent(hidecursor ? -1 : ent, entorient)))
        {
           if(!havesel)
           {
               selchildcount = 0;
               selchildmat = -1;
               sel.s = ivec(0, 0, 0);
           }
        }
        else
        {
            vec w = vec(camdir).mul(wdist+0.05f).add(player->o);
            if(!insideworld(w))
            {
                loopi(3) wdist = min(wdist, ((camdir[i] > 0 ? worldsize : 0) - player->o[i]) / camdir[i]);
                w = vec(camdir).mul(wdist-0.05f).add(player->o);
                if(!insideworld(w))
                {
                    wdist = 0;
                    loopi(3) w[i] = clamp(player->o[i], 0.0f, float(worldsize));
                }
            }
            cube *c = &lookupcube(int(w.x), int(w.y), int(w.z));
            if(gridlookup && !dragging && !moving && !havesel && hmapedit!=1) gridsize = lusize;
            int mag = lusize / gridsize;
            normalizelookupcube(int(w.x), int(w.y), int(w.z));
            if(sdist == 0 || sdist > wdist) rayboxintersect(lu.tovec(), vec(gridsize), player->o, camdir, t=0, orient); // just getting orient
            cur = lu;
            cor = vec(w).mul(2).div(gridsize);
            od = dimension(orient);
            d = dimension(sel.orient);

            if(hmapedit==1 && dimcoord(horient) == (camdir[dimension(horient)]<0))
            {
                hmapsel = isheightmap(horient, dimension(horient), false, c);
                if(hmapsel)
                    od = dimension(orient = horient);
            }

            if(dragging)
            {
                updateselection();
                sel.cx   = min(cor[R[d]], lastcor[R[d]]);
                sel.cy   = min(cor[C[d]], lastcor[C[d]]);
                sel.cxs  = max(cor[R[d]], lastcor[R[d]]);
                sel.cys  = max(cor[C[d]], lastcor[C[d]]);

                if(!selectcorners)
                {
                    sel.cx &= ~1;
                    sel.cy &= ~1;
                    sel.cxs &= ~1;
                    sel.cys &= ~1;
                    sel.cxs -= sel.cx-2;
                    sel.cys -= sel.cy-2;
                }
                else
                {
                    sel.cxs -= sel.cx-1;
                    sel.cys -= sel.cy-1;
                }

                sel.cx  &= 1;
                sel.cy  &= 1;
                havesel = true;
            }
            else if(!havesel)
            {
                sel.o = lu;
                sel.s.x = sel.s.y = sel.s.z = 1;
                sel.cx = sel.cy = 0;
                sel.cxs = sel.cys = 2;
                sel.grid = gridsize;
                sel.orient = orient;
                d = od;
            }

            sel.corner = (cor[R[d]]-(lu[R[d]]*2)/gridsize)+(cor[C[d]]-(lu[C[d]]*2)/gridsize)*2;
            selchildcount = 0;
            selchildmat = -1;
            countselchild(worldroot, ivec(0, 0, 0), worldsize/2);
            if(mag>=1 && selchildcount==1) 
            {
                selchildmat = c->material;
                if(mag>1) selchildcount = -mag;
            }
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // cursors

    lineshader->set();

    renderentselection(player->o, camdir, entmoving!=0);

    boxoutline = outline!=0;

    enablepolygonoffset(GL_POLYGON_OFFSET_LINE);

    if(!moving && !hovering && !hidecursor)
    {
        if(hmapedit==1)
            glColor3ub(0, hmapsel ? 255 : 40, 0);
        else
            glColor3ub(gridhoverselectioncolor>>16, (gridhoverselectioncolor>>8)&0xFF, gridhoverselectioncolor&0xFF);   // Fanatic Edition
        boxs(orient, lu.tovec(), vec(lusize));
    }

    if(havesel || moving)
    {
        d = dimension(sel.orient);
        glColor3ub(gridselectioncolor>>16, (gridselectioncolor>>8)&0xFF, gridselectioncolor&0xFF);   // Fanatic Edition
        boxsgrid(sel.orient, sel.o.tovec(), sel.s.tovec(), sel.grid);
        glColor3ub(200,0,0);
        boxs3D(sel.o.tovec().sub(0.5f*min(gridsize*0.25f, 2.0f)), vec(min(gridsize*0.25f, 2.0f)), 1);
        glColor3ub(grid2Dselectioncolor>>16, (grid2Dselectioncolor>>8)&0xFF, grid2Dselectioncolor&0xFF); // Fanatic Edition
        vec co(sel.o.v), cs(sel.s.v);
        co[R[d]] += 0.5f*(sel.cx*gridsize);
        co[C[d]] += 0.5f*(sel.cy*gridsize);
        cs[R[d]]  = 0.5f*(sel.cxs*gridsize);
        cs[C[d]]  = 0.5f*(sel.cys*gridsize);
        cs[D[d]] *= gridsize;
        boxs(sel.orient, co, cs);
        if(hmapedit==1)
            glColor3ub(0,120,0);
        else
            glColor3ub(grid3Dselectioncolor>>16, (grid3Dselectioncolor>>8)&0xFF, grid3Dselectioncolor&0xFF); // Fanatic Edition
        boxs3D(sel.o.tovec(), sel.s.tovec(), sel.grid);
    }

    disablepolygonoffset(GL_POLYGON_OFFSET_LINE);

    boxoutline = false;

    notextureshader->set();

    glDisable(GL_BLEND);
}

void tryedit()
{
    extern int hidehud;
    if(!editmode || hidehud || mainmenu) return;
    if(blendpaintmode) trypaintblendmap();
}

//////////// ready changes to vertex arrays ////////////

static bool haschanged = false;

void readychanges(const ivec &bbmin, const ivec &bbmax, cube *c, const ivec &cor, int size)
{
    loopoctabox(cor, size, bbmin, bbmax)
    {
        ivec o(i, cor.x, cor.y, cor.z, size);
        if(c[i].ext)
        {
            if(c[i].ext->va)             // removes va s so that octarender will recreate
            {
                int hasmerges = c[i].ext->va->hasmerges;
                destroyva(c[i].ext->va);
                c[i].ext->va = NULL;
                if(hasmerges) invalidatemerges(c[i], o, size, true);
            }
            freeoctaentities(c[i]);
            c[i].ext->tjoints = -1;
        }
        if(c[i].children)
        {
            if(size<=1)
            {
                solidfaces(c[i]);
                discardchildren(c[i], true);
                brightencube(c[i]);
            }
            else readychanges(bbmin, bbmax, c[i].children, o, size/2);
        }
        else brightencube(c[i]);
    }
}

void commitchanges(bool force)
{
    if(!force && !haschanged) return;
    haschanged = false;

    extern vector<vtxarray *> valist;
    int oldlen = valist.length();
    resetclipplanes();
    entitiesinoctanodes();
    inbetweenframes = false;
    octarender();
    inbetweenframes = true;
    setupmaterials(oldlen);
    invalidatepostfx();
    updatevabbs();
    resetblobs();
}

void changed(const block3 &sel, bool commit = true)
{
    if(sel.s.iszero()) return;
    readychanges(ivec(sel.o).sub(1), ivec(sel.s).mul(sel.grid).add(sel.o).add(1), worldroot, ivec(0, 0, 0), worldsize/2);
    haschanged = true;

    if(commit) commitchanges();
}

//////////// copy and undo /////////////
static inline void copycube(const cube &src, cube &dst)
{
    dst = src;
    dst.visible = 0;
    dst.merged = 0;
    dst.ext = NULL; // src cube is responsible for va destruction
    if(src.children)
    {
        dst.children = newcubes(F_EMPTY);
        loopi(8) copycube(src.children[i], dst.children[i]);
    }
}

static inline void pastecube(const cube &src, cube &dst)
{
    discardchildren(dst);
    copycube(src, dst);
}

void blockcopy(const block3 &s, int rgrid, block3 *b)
{
    *b = s;
    cube *q = b->c();
    loopxyz(s, rgrid, copycube(c, *q++));
}

block3 *blockcopy(const block3 &s, int rgrid)
{
    int bsize = sizeof(block3)+sizeof(cube)*s.size();
    if(bsize <= 0 || bsize > (100<<20)) return 0;
    block3 *b = (block3 *)new uchar[bsize];
    blockcopy(s, rgrid, b);
    return b;
}

void freeblock(block3 *b, bool alloced = true)
{
    cube *q = b->c();
    loopi(b->size()) discardchildren(*q++);
    if(alloced) delete[] b;
}

void selgridmap(selinfo &sel, int *g)                           // generates a map of the cube sizes at each grid point
{
    loopxyz(sel, -sel.grid, (*g++ = lusize, (void)c));
}

void freeundo(undoblock *u)
{
    if(!u->numents) freeblock(u->block(), false);
    delete[] (uchar *)u;
}

void pasteundo(undoblock *u)
{
    if(u->numents) pasteundoents(u);
    else
    {
        block3 *b = u->block();
        cube *s = b->c();
        int *g = u->gridmap();
        loopxyz(*b, *g++, pastecube(*s++, c));
    }
}

static inline int undosize(undoblock *u)
{
    if(u->numents) return u->numents*sizeof(undoent);
    else
    {
        block3 *b = u->block();
        cube *q = b->c();
        int size = b->size(), total = size*sizeof(int);
        loopj(size) total += familysize(*q++)*sizeof(cube);
        return total;
    }
}

struct undolist
{
    undoblock *first, *last;

    undolist() : first(NULL), last(NULL) {}

    bool empty() { return !first; }

    void add(undoblock *u)
    {
        u->next = NULL;
        u->prev = last;
        if(!first) first = last = u;
        else
        {
            last->next = u;
            last = u;
        }
    }

    undoblock *popfirst()
    {
        undoblock *u = first;
        first = first->next;
        if(first) first->prev = NULL;
        else last = NULL;
        return u;
    }

    undoblock *poplast()
    {
        undoblock *u = last;
        last = last->prev;
        if(last) last->next = NULL;
        else first = NULL;
        return u;
    }
};

undolist undos, redos;
VARP(undomegs, 0, 5, 100);                           // bounded by n megs
int totalundos = 0;

void pruneundos(int maxremain)                          // bound memory
{
    while(totalundos > maxremain && !undos.empty())
    {
        undoblock *u = undos.popfirst();
        totalundos -= u->size;
        freeundo(u);
    }
    //conoutf(CON_DEBUG, "undo: %d of %d(%%%d)", totalundos, undomegs<<20, totalundos*100/(undomegs<<20));
    while(!redos.empty())
    {
        undoblock *u = redos.popfirst();
        totalundos -= u->size;
        freeundo(u);
    }
}

void clearundos() { pruneundos(0); }

COMMAND(clearundos, "");

undoblock *newundocube(selinfo &s)
{
    int ssize = s.size(),
        selgridsize = ssize*sizeof(int),
        blocksize = sizeof(block3)+ssize*sizeof(cube);
    if(blocksize <= 0 || blocksize > (undomegs<<20)) return NULL;
    undoblock *u = (undoblock *)new uchar[sizeof(undoblock) + blocksize + selgridsize];
    u->numents = 0;
    block3 *b = (block3 *)(u + 1);
    blockcopy(s, -s.grid, b);
    int *g = (int *)((uchar *)b + blocksize);
    selgridmap(s, g);
    return u;
}

void addundo(undoblock *u)
{
    u->size = undosize(u);
    u->timestamp = totalmillis;
    undos.add(u);
    totalundos += u->size;
    pruneundos(undomegs<<20);
}

VARP(nompedit, 0, 0, 1); // Fanatic Edition

void makeundoex(selinfo &s)
{
    if(nompedit && multiplayer(false)) return;
    undoblock *u = newundocube(s);
    if(u) addundo(u);
}

void makeundo()                        // stores state of selected cubes before editing
{
    if(lastsel==sel || sel.s.iszero()) return;
    lastsel=sel;
    makeundoex(sel);
}

void swapundo(undolist &a, undolist &b, const char *s)
{
    if(noedit() || (nompedit && multiplayer())) return;
    if(a.empty()) { conoutf(CON_WARN, "nothing more to %s", s); return; }
    int ts = a.last->timestamp;
    selinfo l = sel;
    while(!a.empty() && ts==a.last->timestamp)
    {
        undoblock *u = a.poplast(), *r;
        if(u->numents) r = copyundoents(u);
        else
        {
            block3 *ub = u->block();
            l.o = ub->o;
            l.s = ub->s;
            l.grid = ub->grid;
            l.orient = ub->orient;
            r = newundocube(l);
        }
        if(r)
        {
            r->size = u->size;
            r->timestamp = totalmillis;
            b.add(r);
        }
        pasteundo(u);
        if(!u->numents) changed(l, false);
        freeundo(u);
    }
    commitchanges();
    if(!hmapsel)
    {
        sel = l;
        reorient();
    }
    forcenextundo();
}

void editundo() { swapundo(undos, redos, "undo"); }
void editredo() { swapundo(redos, undos, "redo"); }

// guard against subdivision
#define protectsel(f) { undoblock *_u = newundocube(sel); f; if(_u) { pasteundo(_u); freeundo(_u); } }

vector<editinfo *> editinfos;
editinfo *localedit = NULL;

template<class B>
static void packcube(cube &c, B &buf)
{
    if(c.children)
    {
        buf.put(0xFF);
        loopi(8) packcube(c.children[i], buf);
    }
    else
    {
        cube data = c;
        lilswap(data.texture, 6);
        buf.put(c.material&0xFF);
        buf.put(c.material>>8);
        buf.put(data.edges, sizeof(data.edges));
        buf.put((uchar *)data.texture, sizeof(data.texture));
    }
}

template<class B>
static bool packblock(block3 &b, B &buf)
{
    if(b.size() <= 0 || b.size() > (1<<20)) return false;
    block3 hdr = b;
    lilswap(hdr.o.v, 3);
    lilswap(hdr.s.v, 3);
    lilswap(&hdr.grid, 1);
    lilswap(&hdr.orient, 1);
    buf.put((const uchar *)&hdr, sizeof(hdr));
    cube *c = b.c();
    loopi(b.size()) packcube(c[i], buf);
    return true;
}

template<class B>
static void unpackcube(cube &c, B &buf)
{
    int mat = buf.get();
    if(mat == 0xFF)
    {
        c.children = newcubes(F_EMPTY);
        loopi(8) unpackcube(c.children[i], buf);
    }
    else
    {
        c.material = mat | (buf.get()<<8);
        buf.get(c.edges, sizeof(c.edges));
        buf.get((uchar *)c.texture, sizeof(c.texture));
        lilswap(c.texture, 6);
    }
}

template<class B>
static bool unpackblock(block3 *&b, B &buf)
{
    if(b) { freeblock(b); b = NULL; }
    block3 hdr;
    buf.get((uchar *)&hdr, sizeof(hdr));
    lilswap(hdr.o.v, 3);
    lilswap(hdr.s.v, 3);
    lilswap(&hdr.grid, 1);
    lilswap(&hdr.orient, 1);
    if(hdr.size() > (1<<20)) return false;
    b = (block3 *)new uchar[sizeof(block3)+hdr.size()*sizeof(cube)];
    *b = hdr;
    cube *c = b->c();
    memset(c, 0, b->size()*sizeof(cube));
    loopi(b->size()) unpackcube(c[i], buf);
    return true;
}

static bool compresseditinfo(const uchar *inbuf, int inlen, uchar *&outbuf, int &outlen)
{
    uLongf len = compressBound(inlen);
    if(len > (1<<20)) return false;
    outbuf = new uchar[len];
    if(compress2((Bytef *)outbuf, &len, (const Bytef *)inbuf, inlen, Z_BEST_COMPRESSION) != Z_OK || len > (1<<16))
    {
        delete[] outbuf;
        outbuf = NULL;
        return false;
    }
    outlen = len;
    return true;
}

static bool uncompresseditinfo(const uchar *inbuf, int inlen, uchar *&outbuf, int &outlen)
{
    if(compressBound(outlen) > (1<<20)) return false;
    uLongf len = outlen;
    outbuf = new uchar[len];
    if(uncompress((Bytef *)outbuf, &len, (const Bytef *)inbuf, inlen) != Z_OK)
    {
        delete[] outbuf;
        outbuf = NULL;
        return false;
    }
    outlen = len;
    return true;
}

bool packeditinfo(editinfo *e, int &inlen, uchar *&outbuf, int &outlen)
{
    vector<uchar> buf;
    if(!e || !e->copy || !packblock(*e->copy, buf)) return false;
    inlen = buf.length();
    return compresseditinfo(buf.getbuf(), buf.length(), outbuf, outlen);
}

bool unpackeditinfo(editinfo *&e, const uchar *inbuf, int inlen, int outlen)
{
    if(e && e->copy) { freeblock(e->copy); e->copy = NULL; }
    uchar *outbuf = NULL;
    if(!uncompresseditinfo(inbuf, inlen, outbuf, outlen)) return false;
    ucharbuf buf(outbuf, outlen);
    if(!e) e = editinfos.add(new editinfo);
    if(!unpackblock(e->copy, buf))
    {
        delete[] outbuf;
        return false;
    }
    delete[] outbuf;
    return true;
}

void freeeditinfo(editinfo *&e)
{
    if(!e) return;
    editinfos.removeobj(e);
    if(e->copy) freeblock(e->copy);
    delete e;
    e = NULL;
}

struct prefabheader
{
    char magic[4];
    int version;
};

struct prefab : editinfo
{
    char *name;

    prefab() : name(NULL) {}
    ~prefab() { DELETEA(name); if(copy) freeblock(copy); }
};

static inline bool htcmp(const char *key, const prefab &b) { return !strcmp(key, b.name); }

static hashset<prefab> prefabs;

void delprefab(char *name)
{
    if(prefabs.remove(name))
        conoutf(CON_INFO, "deleted prefab %s", name);
}
COMMAND(delprefab, "s");

void saveprefab(char *name)
{
    if(!name[0] || noedit(true) || (nompedit && multiplayer())) return;
    prefab *b = prefabs.access(name);
    if(!b)
    {
        b = &prefabs[name];
        b->name = newstring(name);
    }
    if(b->copy) freeblock(b->copy);
    protectsel(b->copy = blockcopy(block3(sel), sel.grid));
    changed(sel);
    defformatstring(filename)(strpbrk(name, "/\\") ? "packages/%s.obr" : "packages/prefab/%s.obr", name);
    path(filename);
    stream *f = opengzfile(filename, "wb");
    if(!f) { conoutf(CON_ERROR, "could not write prefab to %s", filename); return; }
    prefabheader hdr;
    memcpy(hdr.magic, "OEBR", 4);
    hdr.version = 0;
    lilswap(&hdr.version, 1);
    f->write(&hdr, sizeof(hdr));
    streambuf<uchar> s(f);
    if(!packblock(*b->copy, s)) { delete f; conoutf(CON_ERROR, "could not pack prefab %s", filename); return; }
    delete f;
    conoutf(CON_INFO, "wrote prefab file %s", filename);
}
COMMAND(saveprefab, "s");

void pasteblock(block3 &b, selinfo &sel, bool local)
{
    sel.s = b.s;
    int o = sel.orient;
    sel.orient = b.orient;
    cube *s = b.c();
    loopselxyz(if(!isempty(*s) || s->children || s->material != MAT_AIR) pastecube(*s, c); s++); // 'transparent'. old opaque by 'delcube; paste'
    sel.orient = o;
}

void pasteprefab(char *name)
{
    if(!name[0] || noedit() || (nompedit && multiplayer())) return;
    prefab *b = prefabs.access(name);
    if(!b)
    {
        defformatstring(filename)(strpbrk(name, "/\\") ? "packages/%s.obr" : "packages/prefab/%s.obr", name);
        path(filename);
        stream *f = opengzfile(filename, "rb");
        if(!f) { conoutf(CON_ERROR, "could not read prefab %s", filename); return; }
        prefabheader hdr;
        if(f->read(&hdr, sizeof(hdr)) != sizeof(prefabheader) || memcmp(hdr.magic, "OEBR", 4)) { delete f; conoutf(CON_ERROR, "prefab %s has malformatted header", filename); return; }
        lilswap(&hdr.version, 1);
        if(hdr.version != 0) { delete f; conoutf(CON_ERROR, "prefab %s uses unsupported version", filename); return; }
        streambuf<uchar> s(f);
        block3 *copy = NULL;
        if(!unpackblock(copy, s)) { delete f; conoutf(CON_ERROR, "could not unpack prefab %s", filename); return; }
        delete f;
        b = &prefabs[name];
        b->name = newstring(name);
        b->copy = copy;
    }
    pasteblock(*b->copy, sel, true);
}
COMMAND(pasteprefab, "s");

void mpcopy(editinfo *&e, selinfo &sel, bool local)
{
    if(local) game::edittrigger(sel, EDIT_COPY);
    if(e==NULL) e = editinfos.add(new editinfo);
    if(e->copy) freeblock(e->copy);
    e->copy = NULL;
    protectsel(e->copy = blockcopy(block3(sel), sel.grid));
    changed(sel);
}

void mppaste(editinfo *&e, selinfo &sel, bool local)
{
    if(e==NULL) return;
    if(local) game::edittrigger(sel, EDIT_PASTE);
    if(e->copy) pasteblock(*e->copy, sel, local);
}

void copy()
{
    if(noedit(true)) return;
    mpcopy(localedit, sel, true);
}

void pastehilite()
{
    if(!localedit) return;
    sel.s = localedit->copy->s;
    reorient();
    havesel = true;
}

void paste()
{
    if(noedit()) return;
    mppaste(localedit, sel, true);
}

COMMAND(copy, "");
COMMAND(pastehilite, "");
COMMAND(paste, "");
COMMANDN(undo, editundo, "");
COMMANDN(redo, editredo, "");

static VSlot *editingvslot = NULL;

void compacteditvslots()
{
    if(editingvslot && editingvslot->layer) compactvslot(editingvslot->layer);
    loopv(editinfos)
    {
        editinfo *e = editinfos[i];
        compactvslots(e->copy->c(), e->copy->size());
    }
    for(undoblock *u = undos.first; u; u = u->next)
        if(!u->numents)
            compactvslots(u->block()->c(), u->block()->size());
    for(undoblock *u = redos.first; u; u = u->next)
        if(!u->numents)
            compactvslots(u->block()->c(), u->block()->size());
}

#define MAXBRUSH    64
#define MAXBRUSHC   63
#define MAXBRUSH2   32
int brush[MAXBRUSH][MAXBRUSH];
VAR(brushx, 0, MAXBRUSH2, MAXBRUSH);
VAR(brushy, 0, MAXBRUSH2, MAXBRUSH);
bool paintbrush = 0;
int brushmaxx = 0, brushminx = MAXBRUSH;
int brushmaxy = 0, brushminy = MAXBRUSH;

void clearbrush()
{
    memset(brush, 0, sizeof brush);
    brushmaxx = brushmaxy = 0;
    brushminx = brushminy = MAXBRUSH;
    paintbrush = false;
}

void brushvert(int *x, int *y, int *v)
{
    *x += MAXBRUSH2 - brushx + 1; // +1 for automatic padding
    *y += MAXBRUSH2 - brushy + 1;
    if(*x<0 || *y<0 || *x>=MAXBRUSH || *y>=MAXBRUSH) return;
    brush[*x][*y] = clamp(*v, 0, 8);
    paintbrush = paintbrush || (brush[*x][*y] > 0);
    brushmaxx = min(MAXBRUSH-1, max(brushmaxx, *x+1));
    brushmaxy = min(MAXBRUSH-1, max(brushmaxy, *y+1));
    brushminx = max(0,          min(brushminx, *x-1));
    brushminy = max(0,          min(brushminy, *y-1));
}

vector<int> htextures;

COMMAND(clearbrush, "");
COMMAND(brushvert, "iii");
void hmapcancel() { htextures.setsize(0); }
COMMAND(hmapcancel, "");
ICOMMAND(hmapselect, "", (),
    int t = lookupcube(cur.x, cur.y, cur.z).texture[orient];
    int i = htextures.find(t);
    if(i<0)
        htextures.add(t);
    else
        htextures.remove(i);
);

inline bool isheightmap(int o, int d, bool empty, cube *c)
{
    return havesel ||
           (empty && isempty(*c)) ||
           htextures.empty() ||
           htextures.find(c->texture[o]) >= 0;
}

namespace hmap
{
#   define PAINTED     1
#   define NOTHMAP     2
#   define MAPPED      16
    uchar  flags[MAXBRUSH][MAXBRUSH];
    cube   *cmap[MAXBRUSHC][MAXBRUSHC][4];
    int    mapz[MAXBRUSHC][MAXBRUSHC];
    int    map [MAXBRUSH][MAXBRUSH];

    selinfo changes;
    bool selecting;
    int d, dc, dr, dcr, biasup, br, hws, fg;
    int gx, gy, gz, mx, my, mz, nx, ny, nz, bmx, bmy, bnx, bny;
    uint fs;
    selinfo hundo;

    cube *getcube(ivec t, int f)
    {
        t[d] += dcr*f*gridsize;
        if(t[d] > nz || t[d] < mz) return NULL;
        cube *c = &lookupcube(t.x, t.y, t.z, gridsize);
        if(c->children) forcemip(*c, false);
        discardchildren(*c, true);
        if(!isheightmap(sel.orient, d, true, c)) return NULL;
        if     (t.x < changes.o.x) changes.o.x = t.x;
        else if(t.x > changes.s.x) changes.s.x = t.x;
        if     (t.y < changes.o.y) changes.o.y = t.y;
        else if(t.y > changes.s.y) changes.s.y = t.y;
        if     (t.z < changes.o.z) changes.o.z = t.z;
        else if(t.z > changes.s.z) changes.s.z = t.z;
        return c;
    }

    uint getface(cube *c, int d)
    {
        return  0x0f0f0f0f & ((dc ? c->faces[d] : 0x88888888 - c->faces[d]) >> fs);
    }

    void pushside(cube &c, int d, int x, int y, int z)
    {
        ivec a;
        getcubevector(c, d, x, y, z, a);
        a[R[d]] = 8 - a[R[d]];
        setcubevector(c, d, x, y, z, a);
    }

    void addpoint(int x, int y, int z, int v)
    {
        if(!(flags[x][y] & MAPPED))
          map[x][y] = v + (z*8);
        flags[x][y] |= MAPPED;
    }

    void select(int x, int y, int z)
    {
        if((NOTHMAP & flags[x][y]) || (PAINTED & flags[x][y])) return;
        ivec t(d, x+gx, y+gy, dc ? z : hws-z);
        t.shl(gridpower);

        // selections may damage; must makeundo before
        hundo.o = t;
        hundo.o[D[d]] -= dcr*gridsize*2;
        makeundoex(hundo);

        cube **c = cmap[x][y];
        loopk(4) c[k] = NULL;
        c[1] = getcube(t, 0);
        if(!c[1] || !isempty(*c[1]))
        {   // try up
            c[2] = c[1];
            c[1] = getcube(t, 1);
            if(!c[1] || isempty(*c[1])) { c[0] = c[1]; c[1] = c[2]; c[2] = NULL; }
            else { z++; t[d]+=fg; }
        }
        else // drop down
        {
            z--;
            t[d]-= fg;
            c[0] = c[1];
            c[1] = getcube(t, 0);
        }

        if(!c[1] || isempty(*c[1])) { flags[x][y] |= NOTHMAP; return; }

        flags[x][y] |= PAINTED;
        mapz [x][y]  = z;

        if(!c[0]) c[0] = getcube(t, 1);
        if(!c[2]) c[2] = getcube(t, -1);
        c[3] = getcube(t, -2);
        c[2] = !c[2] || isempty(*c[2]) ? NULL : c[2];
        c[3] = !c[3] || isempty(*c[3]) ? NULL : c[3];

        uint face = getface(c[1], d);
        if(face == 0x08080808 && (!c[0] || !isempty(*c[0]))) { flags[x][y] |= NOTHMAP; return; }
        if(c[1]->faces[R[d]] == F_SOLID)   // was single
            face += 0x08080808;
        else                               // was pair
            face += c[2] ? getface(c[2], d) : 0x08080808;
        face += 0x08080808;             // c[3]
        uchar *f = (uchar*)&face;
        addpoint(x,   y,   z, f[0]);
        addpoint(x+1, y,   z, f[1]);
        addpoint(x,   y+1, z, f[2]);
        addpoint(x+1, y+1, z, f[3]);

        if(selecting) // continue to adjacent cubes
        {
            if(x>bmx) select(x-1, y, z);
            if(x<bnx) select(x+1, y, z);
            if(y>bmy) select(x, y-1, z);
            if(y<bny) select(x, y+1, z);
        }
    }

    void ripple(int x, int y, int z, bool force)
    {
        if(force) select(x, y, z);
        if((NOTHMAP & flags[x][y]) || !(PAINTED & flags[x][y])) return;

        bool changed = false;
        int *o[4], best, par, q = 0;
        loopi(2) loopj(2) o[i+j*2] = &map[x+i][y+j];
        #define pullhmap(I, LT, GT, M, N, A) do { \
            best = I; \
            loopi(4) if(*o[i] LT best) best = *o[q = i] - M; \
            par = (best&(~7)) + N; \
            /* dual layer for extra smoothness */ \
            if(*o[q^3] GT par && !(*o[q^1] LT par || *o[q^2] LT par)) { \
                if(*o[q^3] GT par A 8 || *o[q^1] != par || *o[q^2] != par) { \
                    *o[q^3] = (*o[q^3] GT par A 8 ? par A 8 : *o[q^3]); \
                    *o[q^1] = *o[q^2] = par; \
                    changed = true; \
                } \
            /* single layer */ \
            } else { \
                loopj(4) if(*o[j] GT par) { \
                    *o[j] = par; \
                    changed = true; \
                } \
            } \
        } while(0)

        if(biasup)
            pullhmap(0, >, <, 1, 0, -);
        else
            pullhmap(worldsize*8, <, >, 0, 8, +);

        cube **c  = cmap[x][y];
        int e[2][2];
        int notempty = 0;

        loopk(4) if(c[k]) {
            loopi(2) loopj(2) {
                e[i][j] = min(8, map[x+i][y+j] - (mapz[x][y]+3-k)*8);
                notempty |= e[i][j] > 0;
            }
            if(notempty)
            {
                c[k]->texture[sel.orient] = c[1]->texture[sel.orient];
                solidfaces(*c[k]);
                loopi(2) loopj(2)
                {
                    int f = e[i][j];
                    if(f<0 || (f==0 && e[1-i][j]==0 && e[i][1-j]==0))
                    {
                        f=0;
                        pushside(*c[k], d, i, j, 0);
                        pushside(*c[k], d, i, j, 1);
                    }
                    edgeset(cubeedge(*c[k], d, i, j), dc, dc ? f : 8-f);
                }
            }
            else
                emptyfaces(*c[k]);
        }

        if(!changed) return;
        if(x>mx) ripple(x-1, y, mapz[x][y], true);
        if(x<nx) ripple(x+1, y, mapz[x][y], true);
        if(y>my) ripple(x, y-1, mapz[x][y], true);
        if(y<ny) ripple(x, y+1, mapz[x][y], true);

#define DIAGONAL_RIPPLE(a,b,exp) if(exp) { \
            if(flags[x a][ y] & PAINTED) \
                ripple(x a, y b, mapz[x a][y], true); \
            else if(flags[x][y b] & PAINTED) \
                ripple(x a, y b, mapz[x][y b], true); \
        }

        DIAGONAL_RIPPLE(-1, -1, (x>mx && y>my)); // do diagonals because adjacents
        DIAGONAL_RIPPLE(-1, +1, (x>mx && y<ny)); //    won't unless changed
        DIAGONAL_RIPPLE(+1, +1, (x<nx && y<ny));
        DIAGONAL_RIPPLE(+1, -1, (x<nx && y>my));
    }

#define loopbrush(i) for(int x=bmx; x<=bnx+i; x++) for(int y=bmy; y<=bny+i; y++)

    void paint()
    {
        loopbrush(1)
            map[x][y] -= dr * brush[x][y];
    }

    void smooth()
    {
        int sum, div;
        loopbrush(-2)
        {
            sum = 0;
            div = 9;
            loopi(3) loopj(3)
                if(flags[x+i][y+j] & MAPPED)
                    sum += map[x+i][y+j];
                else div--;
            if(div)
                map[x+1][y+1] = sum / div;
        }
    }

    void rippleandset()
    {
        loopbrush(0)
            ripple(x, y, gz, false);
    }

    void run(int dir, int mode)
    {
        d  = dimension(sel.orient);
        dc = dimcoord(sel.orient);
        dcr= dc ? 1 : -1;
        dr = dir>0 ? 1 : -1;
        br = dir>0 ? 0x08080808 : 0;
     //   biasup = mode == dir<0;
        biasup = dir<0;
        bool paintme = paintbrush;
        int cx = (sel.corner&1 ? 0 : -1);
        int cy = (sel.corner&2 ? 0 : -1);
        hws= (worldsize>>gridpower);
        gx = (cur[R[d]] >> gridpower) + cx - MAXBRUSH2;
        gy = (cur[C[d]] >> gridpower) + cy - MAXBRUSH2;
        gz = (cur[D[d]] >> gridpower);
        fs = dc ? 4 : 0;
        fg = dc ? gridsize : -gridsize;
        mx = max(0, -gx); // ripple range
        my = max(0, -gy);
        nx = min(MAXBRUSH-1, hws-gx) - 1;
        ny = min(MAXBRUSH-1, hws-gy) - 1;
        if(havesel)
        {   // selection range
            bmx = mx = max(mx, (sel.o[R[d]]>>gridpower)-gx);
            bmy = my = max(my, (sel.o[C[d]]>>gridpower)-gy);
            bnx = nx = min(nx, (sel.s[R[d]]+(sel.o[R[d]]>>gridpower))-gx-1);
            bny = ny = min(ny, (sel.s[C[d]]+(sel.o[C[d]]>>gridpower))-gy-1);
        }
        if(havesel && mode<0) // -ve means smooth selection
            paintme = false;
        else
        {   // brush range
            bmx = max(mx, brushminx);
            bmy = max(my, brushminy);
            bnx = min(nx, brushmaxx-1);
            bny = min(ny, brushmaxy-1);
        }
        nz = worldsize-gridsize;
        mz = 0;
        hundo.s = ivec(d,1,1,5);
        hundo.orient = sel.orient;
        hundo.grid = gridsize;
        forcenextundo();

        changes.grid = gridsize;
        changes.s = changes.o = cur;
        memset(map, 0, sizeof map);
        memset(flags, 0, sizeof flags);

        selecting = true;
        select(clamp(MAXBRUSH2-cx, bmx, bnx),
               clamp(MAXBRUSH2-cy, bmy, bny),
               dc ? gz : hws - gz);
        selecting = false;
        if(paintme)
            paint();
        else
            smooth();
        rippleandset();                    // pull up points to cubify, and set
        changes.s.sub(changes.o).shr(gridpower).add(1);
        changed(changes);
    }
}

void edithmap(int dir, int mode) {
    if((nompedit && multiplayer()) || !hmapsel) return;
    hmap::run(dir, mode);
}

///////////// main cube edit ////////////////

int bounded(int n) { return n<0 ? 0 : (n>8 ? 8 : n); }

void pushedge(uchar &edge, int dir, int dc)
{
    int ne = bounded(edgeget(edge, dc)+dir);
    edgeset(edge, dc, ne);
    int oe = edgeget(edge, 1-dc);
    if((dir<0 && dc && oe>ne) || (dir>0 && dc==0 && oe<ne)) edgeset(edge, 1-dc, ne);
}

void linkedpush(cube &c, int d, int x, int y, int dc, int dir)
{
    ivec v, p;
    getcubevector(c, d, x, y, dc, v);

    loopi(2) loopj(2)
    {
        getcubevector(c, d, i, j, dc, p);
        if(v==p)
            pushedge(cubeedge(c, d, i, j), dir, dc);
    }
}

static ushort getmaterial(cube &c)
{
    if(c.children)
    {
        ushort mat = getmaterial(c.children[7]);
        loopi(7) if(mat != getmaterial(c.children[i])) return MAT_AIR;
        return mat;
    }
    return c.material;
}

VAR(invalidcubeguard, 0, 1, 1);

void mpeditface(int dir, int mode, selinfo &sel, bool local)
{
    if(mode==1 && (sel.cx || sel.cy || sel.cxs&1 || sel.cys&1)) mode = 0;
    int d = dimension(sel.orient);
    int dc = dimcoord(sel.orient);
    int seldir = dc ? -dir : dir;

    if(local)
        game::edittrigger(sel, EDIT_FACE, dir, mode);

    if(mode==1)
    {
        int h = sel.o[d]+dc*sel.grid;
        if(((dir>0) == dc && h<=0) || ((dir<0) == dc && h>=worldsize)) return;
        if(dir<0) sel.o[d] += sel.grid * seldir;
    }

    if(dc) sel.o[d] += sel.us(d)-sel.grid;
    sel.s[d] = 1;

    loopselxyz(
        if(c.children) solidfaces(c);
        ushort mat = getmaterial(c);
        discardchildren(c, true);
        c.material = mat;
        if(mode==1) // fill command
        {
            if(dir<0)
            {
                solidfaces(c);
                cube &o = blockcube(x, y, 1, sel, -sel.grid);
                loopi(6)
                    c.texture[i] = o.children ? DEFAULT_GEOM : o.texture[i];
            }
            else
                emptyfaces(c);
        }
        else
        {
            uint bak = c.faces[d];
            uchar *p = (uchar *)&c.faces[d];

            if(mode==2)
                linkedpush(c, d, sel.corner&1, sel.corner>>1, dc, seldir); // corner command
            else
            {
                loop(mx,2) loop(my,2)                                       // pull/push edges command
                {
                    if(x==0 && mx==0 && sel.cx) continue;
                    if(y==0 && my==0 && sel.cy) continue;
                    if(x==sel.s[R[d]]-1 && mx==1 && (sel.cx+sel.cxs)&1) continue;
                    if(y==sel.s[C[d]]-1 && my==1 && (sel.cy+sel.cys)&1) continue;
                    if(p[mx+my*2] != ((uchar *)&bak)[mx+my*2]) continue;

                    linkedpush(c, d, mx, my, dc, seldir);
                }
            }

            optiface(p, c);
            if(invalidcubeguard==1 && !isvalidcube(c))
            {
                uint newbak = c.faces[d];
                uchar *m = (uchar *)&bak;
                uchar *n = (uchar *)&newbak;
                loopk(4) if(n[k] != m[k]) // tries to find partial edit that is valid
                {
                    c.faces[d] = bak;
                    c.edges[d*4+k] = n[k];
                    if(isvalidcube(c))
                        m[k] = n[k];
                }
                c.faces[d] = bak;
            }
        }
    );
    if(mode==1 && dir>0)
        sel.o[d] += sel.grid * seldir;
}

void editface(int *dir, int *mode)
{
    if(noedit(moving!=0)) return;
    if(hmapedit!=1)
        mpeditface(*dir, *mode, sel, true);
    else
        edithmap(*dir, *mode);
}

VAR(selectionsurf, 0, 0, 1);

void pushsel(int *dir)
{
    if(noedit(moving!=0)) return;
    int d = dimension(orient);
    int s = dimcoord(orient) ? -*dir : *dir;
    sel.o[d] += s*sel.grid;
    if(selectionsurf==1)
    {
        player->o[d] += s*sel.grid;
        player->resetinterp();
    }
}

void mpdelcube(selinfo &sel, bool local)
{
    if(local) game::edittrigger(sel, EDIT_DELCUBE);
    loopselxyz(discardchildren(c, true); emptyfaces(c));
}

void delcube()
{
    if(noedit()) return;
    mpdelcube(sel, true);
}

COMMAND(pushsel, "i");
COMMAND(editface, "ii");
COMMAND(delcube, "");

int curtexindex = -1, lasttex = 0, lasttexmillis = -1;
int texpaneltimer = 0;
vector<ushort> texmru;

void tofronttex()
{
    int c = curtexindex;
    if(texmru.inrange(c))
    {
        texmru.insert(0, texmru.remove(c));
        curtexindex = -1;
    }
}

selinfo repsel;
int reptex = -1;

struct vslotmap
{
    int index;
    VSlot *vslot;

    vslotmap() {}
    vslotmap(int index, VSlot *vslot) : index(index), vslot(vslot) {}
};
static vector<vslotmap> remappedvslots;

VAR(usevdelta, 1, 0, 0);

static VSlot *remapvslot(int index, const VSlot &ds)
{
    loopv(remappedvslots) if(remappedvslots[i].index == index) return remappedvslots[i].vslot;
    VSlot &vs = lookupvslot(index, false);
    if(vs.index < 0 || vs.index == DEFAULT_SKY) return NULL;
    VSlot *edit = NULL;
    if(usevdelta)
    {
        VSlot ms;
        mergevslot(ms, vs, ds);
        edit = ms.changed ? editvslot(vs, ms) : vs.slot->variants;
    }
    else edit = ds.changed ? editvslot(vs, ds) : vs.slot->variants;
    if(!edit) edit = &vs;
    remappedvslots.add(vslotmap(vs.index, edit));
    return edit;
}

static void remapvslots(cube &c, const VSlot &ds, int orient, bool &findrep, VSlot *&findedit)
{
    if(c.children)
    {
        loopi(8) remapvslots(c.children[i], ds, orient, findrep, findedit);
        return;
    }
    static VSlot ms;
    if(orient<0) loopi(6)
    {
        VSlot *edit = remapvslot(c.texture[i], ds);
        if(edit)
        {
            c.texture[i] = edit->index;
            if(!findedit) findedit = edit;
        }
    }
    else
    {
        int i = visibleorient(c, orient);
        VSlot *edit = remapvslot(c.texture[i], ds);
        if(edit)
        {
            if(findrep)
            {
                if(reptex < 0) reptex = c.texture[i];
                else if(reptex != c.texture[i]) findrep = false;
            }
            c.texture[i] = edit->index;
            if(!findedit) findedit = edit;
        }
    }
}

void edittexcube(cube &c, int tex, int orient, bool &findrep)
{
    if(orient<0) loopi(6) c.texture[i] = tex;
    else
    {
        int i = visibleorient(c, orient);
        if(findrep)
        {
            if(reptex < 0) reptex = c.texture[i];
            else if(reptex != c.texture[i]) findrep = false;
        }
        c.texture[i] = tex;
    }
    if(c.children) loopi(8) edittexcube(c.children[i], tex, orient, findrep);
}

VAR(allfaces, 0, 0, 1);

void mpeditvslot(VSlot &ds, int allfaces, selinfo &sel, bool local)
{
    if(local)
    {
        if(!(lastsel==sel)) tofronttex();
        if(allfaces || !(repsel == sel)) reptex = -1;
        repsel = sel;
    }
    bool findrep = local && !allfaces && reptex < 0;
    VSlot *findedit = NULL;
    editingvslot = &ds;
    loopselxyz(remapvslots(c, ds, allfaces ? -1 : sel.orient, findrep, findedit));
    editingvslot = NULL;
    remappedvslots.setsize(0);
    if(local && findedit)
    {
        lasttex = findedit->index;
        lasttexmillis = totalmillis;
        curtexindex = texmru.find(lasttex);
        if(curtexindex < 0)
        {
            curtexindex = texmru.length();
            texmru.add(lasttex);
        }
    }
}

void vdelta(char *body)
{
    if(noedit() || (nompedit && multiplayer())) return;
    usevdelta++;
    execute(body);
    usevdelta--;
}
COMMAND(vdelta, "s");

void vrotate(int *n)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_ROTATION;
    ds.rotation = usevdelta ? *n : clamp(*n, 0, 5);
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vrotate, "i");

void voffset(int *x, int *y)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_OFFSET;
    ds.xoffset = usevdelta ? *x : max(*x, 0);
    ds.yoffset = usevdelta ? *y : max(*y, 0);
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(voffset, "ii");

void vscroll(float *s, float *t)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_SCROLL;
    ds.scrollS = *s/1000.0f;
    ds.scrollT = *t/1000.0f;
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vscroll, "ff");

void vscale(float *scale)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_SCALE;
    ds.scale = *scale <= 0 ? 1 : (usevdelta ? *scale : clamp(*scale, 1/8.0f, 8.0f));
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vscale, "f");

void vlayer(int *n)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_LAYER;
    ds.layer = vslots.inrange(*n) ? *n : 0;
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vlayer, "i");

void valpha(float *front, float *back)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_ALPHA;
    ds.alphafront = clamp(*front, 0.0f, 1.0f);
    ds.alphaback = clamp(*back, 0.0f, 1.0f);
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(valpha, "ff");

void vcolor(float *r, float *g, float *b)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_COLOR;
    ds.colorscale = vec(clamp(*r, 0.0f, 1.0f), clamp(*g, 0.0f, 1.0f), clamp(*b, 0.0f, 1.0f));
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vcolor, "fff");

void vreset()
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vreset, "");

void vshaderparam(const char *name, float *x, float *y, float *z, float *w)
{
    if(noedit() || (nompedit && multiplayer())) return;
    VSlot ds;
    ds.changed = 1<<VSLOT_SHPARAM;
    if(name[0])
    {
        ShaderParam p = { getshaderparamname(name), SHPARAM_LOOKUP, -1, -1, {*x, *y, *z, *w} };
        ds.params.add(p);
    }
    mpeditvslot(ds, allfaces, sel, true);
}
COMMAND(vshaderparam, "sffff");

// Start: Fanatic Edition
void editent(int *i, int *x, int *y, int *z, int *type, int *attr1, int *attr2, int *attr3, int *attr4, int *attr5)
{
     mpeditent(*i, vec(*x, *y, *z), *type, *attr1, *attr2, *attr3, *attr4, *attr5, true);
}
COMMAND(editent, "iiiiiiiiii");
// End: Fanatic Edition

void mpedittex(int tex, int allfaces, selinfo &sel, bool local)
{
    if(local)
    {
        game::edittrigger(sel, EDIT_TEX, tex, allfaces);
        if(allfaces || !(repsel == sel)) reptex = -1;
        repsel = sel;
    }
    bool findrep = local && !allfaces && reptex < 0;
    loopselxyz(edittexcube(c, tex, allfaces ? -1 : sel.orient, findrep));
}

void filltexlist()
{
    if(texmru.length()!=vslots.length())
    {
        loopvrev(texmru) if(texmru[i]>=vslots.length())
        {
            if(curtexindex > i) curtexindex--;
            else if(curtexindex == i) curtexindex = -1;
            texmru.remove(i);
        }
        loopv(vslots) if(texmru.find(i)<0) texmru.add(i);
    }
}

void compactmruvslots()
{
    remappedvslots.setsize(0);
    loopvrev(texmru)
    {
        if(vslots.inrange(texmru[i]))
        {
            VSlot &vs = *vslots[texmru[i]];
            if(vs.index >= 0)
            {
                texmru[i] = vs.index;
                continue;
            }
        }
        if(curtexindex > i) curtexindex--;
        else if(curtexindex == i) curtexindex = -1;
        texmru.remove(i);
    }
    if(vslots.inrange(lasttex))
    {
        VSlot &vs = *vslots[lasttex];
        lasttex = vs.index >= 0 ? vs.index : 0;
    }
    else lasttex = 0;
    reptex = vslots.inrange(reptex) ? vslots[reptex]->index : -1;
}

void edittex(int i, bool save = true)
{
    lasttex = i;
    lasttexmillis = totalmillis;
    if(save)
    {
        loopvj(texmru) if(texmru[j]==lasttex) { curtexindex = j; break; }
    }
    mpedittex(i, allfaces, sel, true);
}

void edittex_(int *dir)
{
    if(noedit()) return;
    filltexlist();
    if(texmru.empty()) return;
    texpaneltimer = 5000;
    if(!(lastsel==sel)) tofronttex();
    curtexindex = clamp(curtexindex<0 ? 0 : curtexindex+*dir, 0, texmru.length()-1);
    edittex(texmru[curtexindex], false);
}

void gettex()
{
    if(noedit(true)) return;
    filltexlist();
    int tex = -1;
    loopxyz(sel, sel.grid, tex = c.texture[sel.orient]);
    loopv(texmru) if(texmru[i]==tex)
    {
        curtexindex = i;
        tofronttex();
        return;
    }
}

void getcurtex()
{
    if(noedit(true)) return;
    filltexlist();
    int index = curtexindex < 0 ? 0 : curtexindex;
    if(!texmru.inrange(index)) return;
    intret(texmru[index]);
}

void getseltex()
{
    if(noedit(true)) return;
    cube &c = lookupcube(sel.o.x, sel.o.y, sel.o.z, -sel.grid);
    if(c.children || isempty(c)) return;
    intret(c.texture[sel.orient]);
}

void gettexname(int *tex, int *subslot)
{
    if(noedit(true) || *tex<0) return;
    VSlot &vslot = lookupvslot(*tex, false);
    Slot &slot = *vslot.slot;
    if(!slot.sts.inrange(*subslot)) return;
    result(slot.sts[*subslot].name);
}

// Start: Fanatic Edition
COMMANDN(edittex, edittex_, "i");
ICOMMAND(settex, "i", (int *tex), { if(!vslots.inrange(*tex) || noedit()) return; filltexlist(); edittex(*tex); });

COMMAND(gettex, "");
COMMAND(getcurtex, "");
COMMAND(getseltex, "");
ICOMMAND(getreptex, "", (), { if(!noedit()) intret(vslots.inrange(reptex) ? reptex : -1); });
COMMAND(gettexname, "ii");

ICOMMAND(getvalpha, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    VSlot &v = *vslots[*slot];
    defformatstring(str)("%g %g", v.alphafront, v.alphaback);
    result(str);
)

ICOMMAND(getvcolor, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    VSlot &v = *vslots[*slot];
    defformatstring(str)("%g %g %g", v.colorscale.x, v.colorscale.y, v.colorscale.z);
    result(str);
)

ICOMMAND(getvlayer, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    intret(vslots[*slot]->layer);
)

ICOMMAND(getvoffset, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    VSlot &v = *vslots[*slot];
    defformatstring(str)("%i %i", v.xoffset, v.yoffset);
    result(str);
)

ICOMMAND(getvrotate, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    intret(vslots[*slot]->rotation);
)

ICOMMAND(getvscale, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    floatret(vslots[*slot]->scale);
)

ICOMMAND(getvscroll, "i", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    VSlot &v = *vslots[*slot];
    defformatstring(str)("%g %g", v.scrollS * 1000.0, v.scrollT * 1000.0);
    result(str);
)

ICOMMAND(getvshaderparam, "is", (int *slot, const char *param),
    if(noedit(true) || !vslots.inrange(*slot)) return;
    float vals[4]; loopi(4) vals[i] = 0;
    VSlot &v = *vslots[*slot];
    Slot &s = *v.slot;

    loopv(v.params)
    {
        if(!strcmp(v.params[i].name, param))
        {
            loopj(4) vals[j] = v.params[i].val[j];
            goto end;
        }
    }

    loopv(s.params)
    {
        if(!strcmp(s.params[i].name, param))
        {
            loopj(4) vals[j] = s.params[i].val[j];
            goto end;
        }
    }

    end:
    defformatstring(str)("%g %g %g %g", vals[0], vals[1], vals[2], vals[3]);
    result(str);
)

ICOMMAND(getvshaderparam, "is", (int *slot),
    if(noedit(true) || !vslots.inrange(*slot)) return;
)
// End: Fanatic Edition

void replacetexcube(cube &c, int oldtex, int newtex)
{
    loopi(6) if(c.texture[i] == oldtex) c.texture[i] = newtex;
    if(c.children) loopi(8) replacetexcube(c.children[i], oldtex, newtex);
}

void mpreplacetex(int oldtex, int newtex, bool insel, selinfo &sel, bool local)
{
    if(local) game::edittrigger(sel, EDIT_REPLACE, oldtex, newtex, insel ? 1 : 0);
    if(insel)
    {
        loopselxyz(replacetexcube(c, oldtex, newtex));
    }
    else
    {
        loopi(8) replacetexcube(worldroot[i], oldtex, newtex);
    }
    allchanged();
}

void replace(bool insel)
{
    if(noedit()) return;
    if(reptex < 0) { conoutf(CON_ERROR, "can only replace after a texture edit"); return; }
    mpreplacetex(reptex, lasttex, insel, sel, true);
}

ICOMMAND(replace, "", (), replace(false));
ICOMMAND(replacesel, "", (), replace(true));

// Start: Fanatic Edition
ICOMMAND(replacetex, "iii", (int *oldtex, int *newtex, int *insel), {
    if(noedit()) return;
    mpreplacetex(*oldtex, *newtex, *insel!=0, sel, true);
});
// End: Fanatic Edition

uint dflip(uint face) { return face==F_EMPTY ? face : 0x88888888 - (((face&0xF0F0F0F0)>>4) | ((face&0x0F0F0F0F)<<4)); }
uint cflip(uint face) { return ((face&0xFF00FF00)>>8) | ((face&0x00FF00FF)<<8); }
uint rflip(uint face) { return ((face&0xFFFF0000)>>16)| ((face&0x0000FFFF)<<16); }
uint mflip(uint face) { return (face&0xFF0000FF) | ((face&0x00FF0000)>>8) | ((face&0x0000FF00)<<8); }

void flipcube(cube &c, int d)
{
    swap(c.texture[d*2], c.texture[d*2+1]);
    c.faces[D[d]] = dflip(c.faces[D[d]]);
    c.faces[C[d]] = cflip(c.faces[C[d]]);
    c.faces[R[d]] = rflip(c.faces[R[d]]);
    if(c.children)
    {
        loopi(8) if(i&octadim(d)) swap(c.children[i], c.children[i-octadim(d)]);
        loopi(8) flipcube(c.children[i], d);
    }
}

void rotatequad(cube &a, cube &b, cube &c, cube &d)
{
    cube t = a; a = b; b = c; c = d; d = t;
}

void rotatecube(cube &c, int d)   // rotates cube clockwise. see pics in cvs for help.
{
    c.faces[D[d]] = cflip (mflip(c.faces[D[d]]));
    c.faces[C[d]] = dflip (mflip(c.faces[C[d]]));
    c.faces[R[d]] = rflip (mflip(c.faces[R[d]]));
    swap(c.faces[R[d]], c.faces[C[d]]);

    swap(c.texture[2*R[d]], c.texture[2*C[d]+1]);
    swap(c.texture[2*C[d]], c.texture[2*R[d]+1]);
    swap(c.texture[2*C[d]], c.texture[2*C[d]+1]);

    if(c.children)
    {
        int row = octadim(R[d]);
        int col = octadim(C[d]);
        for(int i=0; i<=octadim(d); i+=octadim(d)) rotatequad
        (
            c.children[i+row],
            c.children[i],
            c.children[i+col],
            c.children[i+col+row]
        );
        loopi(8) rotatecube(c.children[i], d);
    }
}

void mpflip(selinfo &sel, bool local)
{
    if(local) 
    { 
        game::edittrigger(sel, EDIT_FLIP);
        makeundo();
    }
    int zs = sel.s[dimension(sel.orient)];
    loopxy(sel)
    {
        loop(z,zs) flipcube(selcube(x, y, z), dimension(sel.orient));
        loop(z,zs/2)
        {
            cube &a = selcube(x, y, z);
            cube &b = selcube(x, y, zs-z-1);
            swap(a, b);
        }
    }
    changed(sel);
}

void flip()
{
    if(noedit()) return;
    mpflip(sel, true);
}

void mprotate(int cw, selinfo &sel, bool local)
{
    if(local) 
    {
        game::edittrigger(sel, EDIT_ROTATE, cw);
        makeundo();
    }
    int d = dimension(sel.orient);
    if(!dimcoord(sel.orient)) cw = -cw;
    int m = sel.s[C[d]] < sel.s[R[d]] ? C[d] : R[d];
    int ss = sel.s[m] = max(sel.s[R[d]], sel.s[C[d]]);
    loop(z,sel.s[D[d]]) loopi(cw>0 ? 1 : 3)
    {
        loopxy(sel) rotatecube(selcube(x,y,z), d);
        loop(y,ss/2) loop(x,ss-1-y*2) rotatequad
        (
            selcube(ss-1-y, x+y, z),
            selcube(x+y, y, z),
            selcube(y, ss-1-x-y, z),
            selcube(ss-1-x-y, ss-1-y, z)
        );
    }
    changed(sel);
}

void rotate(int *cw)
{
    if(noedit()) return;
    mprotate(*cw, sel, true);
}

COMMAND(flip, "");
COMMAND(rotate, "i");

enum { EDITMATF_EMPTY = 0x10000, EDITMATF_NOTEMPTY = 0x20000, EDITMATF_SOLID = 0x30000, EDITMATF_NOTSOLID = 0x40000 };
static const struct { const char *name; int filter; } editmatfilters[] = 
{ 
    { "empty", EDITMATF_EMPTY }, { "notempty", EDITMATF_NOTEMPTY }, { "solid", EDITMATF_SOLID }, { "notsolid", EDITMATF_NOTSOLID }
};

void setmat(cube &c, ushort mat, ushort matmask, ushort filtermat, ushort filtermask, int filtergeom)
{
    if(c.children)
        loopi(8) setmat(c.children[i], mat, matmask, filtermat, filtermask, filtergeom);
    else if((c.material&filtermask) == filtermat)
    {
        switch(filtergeom)
        {
            case EDITMATF_EMPTY: if(isempty(c)) break; return;
            case EDITMATF_NOTEMPTY: if(!isempty(c)) break; return;
            case EDITMATF_SOLID: if(isentirelysolid(c)) break; return;
            case EDITMATF_NOTSOLID: if(!isentirelysolid(c)) break; return;
        }
        if(mat!=MAT_AIR)
        {
            c.material &= matmask;
            c.material |= mat;
        }
        else c.material = MAT_AIR;
    }
}

void mpeditmat(int matid, int filter, selinfo &sel, bool local)
{
    if(local) game::edittrigger(sel, EDIT_MAT, matid, filter);

    ushort filtermat = 0, filtermask = 0, matmask;
    int filtergeom = 0;
    if(filter >= 0)
    {
        filtermat = filter&0xFFFF;
        filtermask = filtermat&(MATF_VOLUME|MATF_INDEX) ? MATF_VOLUME|MATF_INDEX : (filtermat&MATF_CLIP ? MATF_CLIP : filtermat);
        filtergeom = filter&~0xFFFF;
    }
    if(matid < 0)
    {
        matid = 0;
        matmask = filtermask;
        if(isclipped(filtermat&MATF_VOLUME)) matmask &= ~MATF_CLIP;
        if(isdeadly(filtermat&MATF_VOLUME)) matmask &= ~MAT_DEATH;
    }
    else
    {
        matmask = matid&(MATF_VOLUME|MATF_INDEX) ? 0 : (matid&MATF_CLIP ? ~MATF_CLIP : ~matid);
        if(isclipped(matid&MATF_VOLUME)) matid |= MAT_CLIP;
        if(isdeadly(matid&MATF_VOLUME)) matid |= MAT_DEATH;
    }
    loopselxyz(setmat(c, matid, matmask, filtermat, filtermask, filtergeom));
}

void editmat(char *name, char *filtername)
{
    if(noedit()) return;
    int filter = -1;
    if(filtername[0])
    {
        loopi(sizeof(editmatfilters)/sizeof(editmatfilters[0])) if(!strcmp(editmatfilters[i].name, filtername)) { filter = editmatfilters[i].filter; break; }
        if(filter < 0) filter = findmaterial(filtername);
        if(filter < 0)
        {
            conoutf(CON_ERROR, "unknown material \"%s\"", filtername); 
            return; 
        }
    }
    int id = -1;
    if(name[0] || filter < 0)
    {
        id = findmaterial(name);
        if(id<0) { conoutf(CON_ERROR, "unknown material \"%s\"", name); return; }
    }
    mpeditmat(id, filter, sel, true);
}

COMMAND(editmat, "ss");

extern int menudistance, menuautoclose;

VARP(texguiwidth, 1, 12, 1000);
VARP(texguiheight, 1, 8, 1000);
VARP(texguitime, 0, 25, 1000);

static int lastthumbnail = 0;

VARP(texgui2d, 0, 1, 1);

struct texturegui : g3d_callback
{
    bool menuon;
    vec menupos;
    int menustart, menutab;

    texturegui() : menustart(-1) {}

    void gui(g3d_gui &g, bool firstpass)
    {
        int origtab = menutab, numtabs = max((slots.length() + texguiwidth*texguiheight - 1)/(texguiwidth*texguiheight), 1);
        g.start(menustart, 0.04f, &menutab);
        loopi(numtabs)
        {
            g.tab(!i ? "Textures" : NULL, 0x50CFE5);
            if(i+1 != origtab) continue; //don't load textures on non-visible tabs!
            loop(h, texguiheight)
            {
                g.pushlist();
                loop(w, texguiwidth)
                {
                    extern VSlot dummyvslot;
                    int ti = (i*texguiheight+h)*texguiwidth+w;
                    if(ti<slots.length())
                    {
                        Slot &slot = lookupslot(ti, false);
                        VSlot &vslot = *slot.variants;
                        if(slot.sts.empty()) continue;
                        else if(!slot.loaded && !slot.thumbnail)
                        {
                            if(totalmillis-lastthumbnail<texguitime)
                            {
                                g.texture(dummyvslot, 1.0, false); //create an empty space
                                continue;
                            }
                            loadthumbnail(slot);
                            lastthumbnail = totalmillis;
                        }
                        if(g.texture(vslot, 1.0f, true)&G3D_UP && (slot.loaded || slot.thumbnail!=notexture))
                            edittex(vslot.index);
                    }
                    else
                    {
                        g.texture(dummyvslot, 1.0, false); //create an empty space
                    }
                }
                g.poplist();
            }
        }
        g.end();
    }

    void showtextures(bool on)
    {
        if(on != menuon && (menuon = on))
        {
            if(menustart <= lasttexmillis)
                menutab = 1+clamp(lookupvslot(lasttex, false).slot->index, 0, slots.length()-1)/(texguiwidth*texguiheight);
            menupos = menuinfrontofplayer();
            menustart = starttime();
        }
    }

    void show()
    {
        if(!menuon) return;
        filltexlist();
        extern int usegui2d;
        if(!editmode || ((!texgui2d || !usegui2d) && camera1->o.dist(menupos) > menuautoclose)) menuon = false;
        else g3d_addgui(this, menupos, texgui2d ? GUI_2D : 0);
    }
} gui;

void g3d_texturemenu()
{
    gui.show();
}

void showtexgui(int *n)
{
    if(!editmode) { conoutf(CON_ERROR, "\f9FanEd\f7::showtexgui: \f3#error: \f7operation only allowed in edit mode"); return; } // Fanatic Edition
    gui.showtextures(*n==0 ? !gui.menuon : *n==1);
}

// 0/noargs = toggle, 1 = on, other = off - will autoclose if too far away or exit editmode
COMMAND(showtexgui, "i");

void rendertexturepanel(int w, int h)
{
    if((texpaneltimer -= curtime)>0 && editmode)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPushMatrix();
        glScalef(h/1800.0f, h/1800.0f, 1);
        int y = 50, gap = 10;

        SETSHADER(rgbonly);

        loopi(7)
        {
            int s = (i == 3 ? 285 : 220), ti = curtexindex+i-3;
            if(texmru.inrange(ti))
            {
                VSlot &vslot = lookupvslot(texmru[ti]), *layer = NULL;
                Slot &slot = *vslot.slot;
                Texture *tex = slot.sts.empty() ? notexture : slot.sts[0].t, *glowtex = NULL, *layertex = NULL;
                if(slot.texmask&(1<<TEX_GLOW))
                {
                    loopvj(slot.sts) if(slot.sts[j].type==TEX_GLOW) { glowtex = slot.sts[j].t; break; }
                }
                if(vslot.layer)
                {
                    layer = &lookupvslot(vslot.layer);
                    layertex = layer->slot->sts.empty() ? notexture : layer->slot->sts[0].t;
                }
                float sx = min(1.0f, tex->xs/(float)tex->ys), sy = min(1.0f, tex->ys/(float)tex->xs);
                int x = w*1800/h-s-50, r = s;
                float tc[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
                float xoff = vslot.xoffset, yoff = vslot.yoffset;
                if(vslot.rotation)
                {
                    if((vslot.rotation&5) == 1) { swap(xoff, yoff); loopk(4) swap(tc[k][0], tc[k][1]); }
                    if(vslot.rotation >= 2 && vslot.rotation <= 4) { xoff *= -1; loopk(4) tc[k][0] *= -1; }
                    if(vslot.rotation <= 2 || vslot.rotation == 5) { yoff *= -1; loopk(4) tc[k][1] *= -1; }
                }
                loopk(4) { tc[k][0] = tc[k][0]/sx - xoff/tex->xs; tc[k][1] = tc[k][1]/sy - yoff/tex->ys; }
                glBindTexture(GL_TEXTURE_2D, tex->id);
                loopj(glowtex ? 3 : 2)
                {
                    if(j < 2) glColor4f(j*vslot.colorscale.x, j*vslot.colorscale.y, j*vslot.colorscale.z, texpaneltimer/1000.0f);
                    else
                    {
                        glBindTexture(GL_TEXTURE_2D, glowtex->id);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                        glColor4f(vslot.glowcolor.x, vslot.glowcolor.y, vslot.glowcolor.z, texpaneltimer/1000.0f);
                    }
                    glBegin(GL_TRIANGLE_STRIP);
                    glTexCoord2fv(tc[0]); glVertex2f(x,   y);
                    glTexCoord2fv(tc[1]); glVertex2f(x+r, y);
                    glTexCoord2fv(tc[3]); glVertex2f(x,   y+r);
                    glTexCoord2fv(tc[2]); glVertex2f(x+r, y+r);
                    glEnd();
                    xtraverts += 4;
                    if(j==1 && layertex)
                    {
                        glColor4f(layer->colorscale.x, layer->colorscale.y, layer->colorscale.z, texpaneltimer/1000.0f);
                        glBindTexture(GL_TEXTURE_2D, layertex->id);
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2fv(tc[0]); glVertex2f(x+r/2, y+r/2);
                        glTexCoord2fv(tc[1]); glVertex2f(x+r,   y+r/2);
                        glTexCoord2fv(tc[3]); glVertex2f(x+r/2, y+r);
                        glTexCoord2fv(tc[2]); glVertex2f(x+r,   y+r);
                        glEnd();
                        xtraverts += 4;
                    }
                    if(!j)
                    {
                        r -= 10;
                        x += 5;
                        y += 5;
                    }
                    else if(j == 2) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
            }
            y += s+gap;
        }

        defaultshader->set();

        glPopMatrix();
    }
}

// Start: Fanatic Edition: »Marching Cubes« by Wrack;

static int bendTable[256][24] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0},
    {8,1,8,1,8,8,8,8,0,0,0,0,3,0,0,0,2,0,2,0,2,0,2,0},
    {8,0,8,0,8,0,8,0,1,0,0,0,3,0,0,0,1,0,2,0,1,0,2,0},
    {2,0,2,0,0,0,0,0,8,1,8,8,8,8,8,8,3,0,3,0,3,0,3,0},
    {1,0,2,0,0,0,0,0,8,0,8,0,8,0,8,0,1,0,1,0,3,0,3,0},
    {8,1,2,0,8,8,0,0,1,0,8,0,8,3,8,0,2,0,2,0,3,0,3,0},
    {8,0,8,2,8,0,8,0,8,0,8,0,3,0,0,0,1,0,2,0,3,0,2,0},
    {8,2,8,2,8,8,8,8,8,8,8,8,8,3,8,8,4,0,4,0,4,0,4,0},
    {8,1,2,0,8,0,8,0,1,0,0,0,8,3,8,8,1,0,4,0,1,0,4,0},
    {8,1,8,2,8,8,8,8,8,0,8,0,8,0,8,0,2,0,2,0,4,0,4,0},
    {8,0,8,2,8,0,8,8,8,1,8,0,8,0,8,0,1,0,2,0,4,0,4,0},
    {8,0,8,0,8,0,8,0,8,1,8,8,8,3,8,8,3,0,4,0,3,0,4,0},
    {1,0,8,0,0,0,8,0,8,0,8,0,3,0,8,0,1,0,1,0,3,0,4,0},
    {1,0,8,0,8,0,8,0,8,1,8,8,8,0,8,0,3,0,2,0,3,0,4,0},
    {8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,1,0,2,0,3,0,4,0},
    {0,0,0,0,3,0,3,0,0,0,2,0,0,0,0,0,8,1,8,1,8,1,8,1},
    {1,0,1,0,3,0,3,0,1,0,2,0,0,0,0,0,8,0,8,0,8,0,8,0},
    {1,0,8,0,8,3,8,0,2,0,2,0,3,0,3,0,8,1,2,0,8,8,0,0},
    {8,0,8,0,3,0,0,0,1,0,2,0,3,0,2,0,8,0,8,2,8,0,8,0},
    {2,0,2,0,3,0,3,0,8,1,2,0,8,8,0,0,1,0,8,0,8,3,8,0},
    {1,0,2,0,3,0,2,0,8,0,8,2,8,0,8,0,8,0,8,0,3,0,0,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,0,2,0,8,3,0,0,8,0,2,0,8,3,2,0,8,0,2,0,8,3,8,0},
    {8,0,8,2,3,0,8,0,8,0,2,0,8,3,8,0,8,1,8,0,8,0,4,0},
    {8,1,2,0,8,3,8,0,1,0,2,0,8,3,8,8,8,0,4,0,8,0,4,0},
    {8,1,8,2,3,0,0,0,8,0,2,0,8,0,2,0,1,0,8,2,8,0,8,4},
    {8,0,2,0,8,3,8,0,1,0,8,2,8,0,8,0,8,0,2,0,8,0,4,0},
    {8,0,8,0,3,0,3,0,8,1,2,0,8,3,0,0,1,0,8,0,8,3,8,4},
    {1,0,8,0,3,0,8,0,8,0,8,2,3,0,8,0,8,0,8,0,3,0,8,4},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,0,8,0,3,0,0,0,8,0,2,0,8,0,2,0,8,0,8,2,8,3,8,4},
    {8,8,8,8,8,3,8,8,0,0,4,0,0,0,4,0,8,2,8,2,8,2,8,2},
    {8,1,8,0,3,0,8,0,1,0,1,0,4,0,4,0,1,0,8,2,0,0,8,8},
    {8,1,8,1,8,3,8,3,0,0,0,0,3,0,4,0,8,0,8,0,8,0,8,0},
    {8,0,8,0,8,3,8,8,1,0,4,0,3,0,4,0,8,1,8,0,8,0,8,0},
    {8,0,2,0,8,3,8,0,8,1,8,0,8,0,4,0,8,0,8,2,3,0,8,0},
    {1,0,2,0,8,3,8,8,8,0,4,0,8,0,4,0,8,1,2,0,8,3,8,0},
    {8,1,2,0,8,3,0,0,1,0,8,0,8,3,8,4,8,0,8,0,3,0,3,0},
    {8,0,8,2,3,0,8,0,8,0,8,0,3,0,8,4,1,0,8,0,3,0,8,0},
    {8,2,8,2,8,3,8,3,8,8,0,0,8,3,4,0,8,0,2,0,8,0,8,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,1,8,2,8,3,8,2,8,0,8,0,8,0,8,4,8,0,8,0,0,0,4,0},
    {8,0,2,0,8,3,8,3,1,0,0,0,8,0,8,4,8,1,8,0,8,0,4,0},
    {8,0,8,0,8,3,8,3,8,1,0,0,8,3,4,0,8,0,2,0,8,3,8,4},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,1,8,0,8,3,8,0,1,0,8,0,8,0,8,4,8,0,8,0,8,3,4,0},
    {8,0,8,0,8,3,8,8,8,0,4,0,8,0,4,0,8,1,8,0,8,3,8,4},
    {8,0,8,0,8,0,8,0,0,0,2,0,0,0,4,0,8,1,8,2,8,1,8,2},
    {1,0,0,0,8,0,8,0,1,0,2,0,1,0,4,0,8,0,2,0,8,0,8,0},
    {8,1,8,8,8,0,8,0,3,0,2,0,3,0,4,0,1,0,8,0,8,0,8,0},
    {8,0,8,0,8,0,8,0,1,0,2,0,3,0,4,0,8,0,8,0,8,0,8,0},
    {2,0,2,0,8,0,8,0,8,1,2,0,8,8,4,0,1,0,2,0,8,3,8,0},
    {1,0,2,0,8,0,8,0,8,0,2,0,8,0,8,4,8,0,2,0,8,3,8,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {8,0,2,0,8,0,0,0,8,0,8,2,8,3,8,4,8,0,8,0,3,0,3,0},
    {8,2,8,2,8,0,8,0,8,8,2,0,8,3,4,0,1,0,2,0,8,0,8,4},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {8,1,8,2,8,0,8,0,8,0,8,2,8,0,4,0,1,0,8,0,8,0,8,4},
    {8,0,8,2,8,0,8,8,8,1,8,2,8,0,8,4,8,0,8,0,4,0,4,0},
    {8,0,8,0,8,0,8,0,1,0,8,2,3,0,8,4,8,1,8,2,3,0,4,0},
    {8,1,8,0,8,0,8,0,8,0,8,2,3,0,8,4,8,0,8,2,3,0,4,0},
    {1,0,8,0,8,0,8,0,8,1,2,0,8,0,4,0,1,0,8,0,8,3,8,4},
    {8,0,8,0,8,0,8,0,8,0,8,2,8,0,8,4,8,0,8,0,3,0,4,0},
    {0,0,0,0,0,0,4,0,8,8,8,2,8,8,8,2,8,3,8,3,8,3,8,3},
    {1,0,1,0,4,0,4,0,1,0,8,2,0,0,8,8,8,1,8,0,3,0,8,0},
    {8,1,8,0,8,0,4,0,8,0,8,2,3,0,8,0,8,0,2,0,8,3,8,0},
    {8,0,8,0,4,0,4,0,1,0,8,2,3,0,8,8,8,1,8,2,3,0,8,0},
    {2,0,2,0,4,0,4,0,8,1,8,2,8,8,8,8,8,0,8,0,8,0,8,0},
    {1,0,2,0,1,0,4,0,8,0,2,0,8,0,8,0,1,0,0,0,8,0,8,0},
    {8,1,2,0,8,8,4,0,1,0,2,0,8,3,8,0,2,0,2,0,8,0,8,0},
    {8,0,2,0,8,0,4,0,8,0,2,0,8,3,8,0,1,0,8,2,8,0,8,0},
    {8,0,2,0,8,0,8,4,8,2,8,2,8,3,8,3,8,8,0,0,8,3,4,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,1,8,2,0,0,4,0,8,0,8,2,8,0,8,2,8,0,8,2,3,0,8,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,0,8,0,0,0,4,0,8,1,8,2,8,3,8,2,8,0,8,0,8,0,8,4},
    {8,1,8,0,4,0,4,0,8,0,2,0,8,3,8,8,1,0,8,0,8,0,8,4},
    {1,0,8,0,8,0,8,4,8,1,8,2,8,0,8,0,8,0,8,2,8,0,4,0},
    {8,0,8,0,4,0,4,0,8,0,8,2,8,0,8,8,8,1,8,2,8,0,8,4},
    {0,0,0,0,3,0,4,0,8,0,8,0,8,0,8,0,8,1,8,1,8,3,8,3},
    {1,0,4,0,3,0,4,0,8,1,8,0,8,0,8,0,8,0,8,0,8,3,8,8},
    {8,1,8,8,3,0,4,0,3,0,8,0,3,0,8,0,1,0,8,2,3,0,8,0},
    {8,0,8,0,3,0,8,4,1,0,8,0,3,0,8,0,8,0,8,2,3,0,8,0},
    {3,0,2,0,3,0,4,0,1,0,8,0,8,0,8,0,8,1,8,8,8,0,8,0},
    {1,0,2,0,3,0,4,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,0,8,2,8,3,8,4,8,0,8,0,3,0,3,0,8,0,2,0,8,0,0,0},
    {8,8,8,2,3,0,4,0,8,3,8,0,8,3,8,0,1,0,8,0,3,0,8,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,0,2,0,8,3,8,4,8,0,8,0,8,0,8,0,8,1,2,0,8,3,4,0},
    {8,0,8,2,3,0,4,0,8,1,8,0,8,0,8,0,8,0,8,2,3,0,8,4},
    {8,0,8,0,3,0,4,0,8,1,8,0,3,0,8,0,1,0,8,0,8,0,8,4},
    {8,1,8,0,8,3,8,4,8,0,8,0,8,3,8,3,8,0,0,0,8,0,4,0},
    {8,1,8,0,3,0,4,0,1,0,8,0,8,0,8,0,1,0,8,2,8,0,8,4},
    {8,0,8,0,8,3,8,4,8,0,8,0,8,0,8,0,8,0,2,0,8,0,4,0},
    {8,0,8,0,3,0,8,4,8,8,8,2,0,0,4,0,8,3,8,2,8,3,8,2},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {8,1,0,0,8,3,4,0,8,0,2,0,8,3,8,4,8,0,8,0,8,3,8,3},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,8,2,0,8,3,4,0,1,0,2,0,8,0,8,4,8,2,8,2,8,0,8,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,0,8,2,3,0,8,4,8,1,8,2,3,0,4,0,8,0,8,0,8,0,8,0},
    {8,0,8,2,3,0,8,4,8,0,8,2,3,0,4,0,8,1,8,0,8,0,8,0},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {1,0,8,0,3,0,8,4,8,1,8,2,8,0,4,0,8,0,8,0,8,0,8,4},
    {8,0,8,0,8,3,8,4,8,0,2,0,8,0,8,4,8,1,8,0,8,0,8,4},
    {8,0,8,0,8,0,8,4,8,0,8,0,0,0,4,0,8,1,8,2,8,3,8,2},
    {8,1,0,0,8,0,4,0,1,0,8,0,1,0,8,4,8,0,8,2,3,0,8,0},
    {1,0,8,0,8,0,8,4,8,0,8,0,3,0,4,0,8,1,8,0,3,0,8,0},
    {8,0,0,0,8,0,4,0,8,1,8,0,8,3,8,4,8,0,8,0,8,3,8,3},
    {8,0,2,0,8,0,4,0,1,0,8,0,8,0,8,4,8,1,2,0,8,0,8,0},
    {8,1,8,2,8,0,8,4,8,0,8,0,4,0,4,0,8,0,8,2,8,0,8,8},
    {8,1,2,0,8,0,4,0,1,0,8,0,8,3,8,4,1,0,8,0,8,0,8,0},
    {8,0,2,0,8,0,4,0,8,0,8,0,8,3,8,4,8,0,8,0,8,0,8,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {8,1,8,2,8,0,4,0,8,0,8,0,8,0,8,4,1,0,8,0,3,0,8,4},
    {8,0,2,0,8,0,8,4,8,1,8,0,8,0,8,4,8,0,8,0,3,0,4,0},
    {8,0,8,0,8,0,8,4,8,1,8,0,8,3,4,0,1,0,2,0,8,0,8,4},
    {8,1,8,0,8,0,8,4,8,0,8,0,8,3,8,4,8,0,2,0,8,0,8,4},
    {1,0,8,0,8,0,8,4,1,0,8,0,8,0,8,4,8,1,8,0,8,0,4,0},
    {8,0,8,0,8,0,8,4,8,0,8,0,8,0,8,4,8,0,8,0,8,0,4,0},
    {8,8,8,8,8,4,8,4,8,8,8,8,8,8,8,4,8,4,8,4,8,4,8,4},
    {1,0,8,0,8,0,8,4,1,0,8,0,8,0,8,4,1,0,8,0,8,0,8,4},
    {8,1,8,1,8,4,8,4,0,0,8,8,3,0,8,4,8,0,8,2,8,0,4,0},
    {8,0,8,0,8,4,8,4,1,0,8,8,3,0,8,4,8,1,8,2,8,0,4,0},
    {8,0,8,2,8,0,4,0,8,1,8,1,8,4,8,4,0,0,8,8,3,0,8,4},
    {1,0,2,0,8,8,8,4,8,0,8,4,8,0,8,4,8,1,8,0,8,3,4,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,8,8,2,8,8,8,4,8,3,8,4,8,3,8,4,8,0,8,0,8,0,8,0},
    {8,1,2,0,8,0,4,0,1,0,0,0,8,3,8,4,1,0,8,0,1,0,8,0},
    {8,1,8,2,8,1,8,4,8,0,8,0,8,0,4,0,0,0,2,0,8,0,8,0},
    {8,0,8,2,8,0,8,4,8,1,8,0,8,0,4,0,8,1,2,0,8,0,8,0},
    {8,0,8,0,8,8,8,4,8,1,8,4,8,3,8,4,8,0,8,0,8,3,8,0},
    {8,1,8,0,8,0,4,0,8,0,8,0,8,3,8,4,8,1,8,0,3,0,8,0},
    {8,1,8,0,8,0,4,0,1,0,8,8,8,0,8,4,3,0,8,2,3,0,8,0},
    {8,0,8,0,8,8,8,4,8,0,8,4,8,0,8,4,8,1,8,2,8,3,8,0},
    {8,0,8,0,8,3,4,0,0,0,2,0,8,8,8,4,8,1,8,4,8,1,8,4},
    {1,0,8,8,3,0,8,4,8,1,8,2,8,0,4,0,8,0,8,0,8,4,8,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {0,0,8,2,3,0,8,4,8,0,8,2,3,0,4,0,8,1,8,1,8,0,8,0},
    {1,0,8,2,3,0,8,4,8,1,8,2,3,0,4,0,8,0,8,0,8,0,8,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {8,0,2,0,8,3,4,0,1,0,2,0,8,0,8,4,8,0,8,2,8,0,8,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {1,0,8,0,3,0,8,4,8,0,8,2,3,0,4,0,8,0,8,0,8,3,8,0},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,0,8,0,8,3,4,0,8,0,8,2,8,0,8,4,8,0,8,2,8,3,8,0},
    {8,8,8,8,8,3,8,4,8,0,8,0,8,0,8,0,8,2,8,2,8,4,8,4},
    {1,0,0,0,8,3,8,4,1,0,8,0,1,0,8,0,8,1,2,0,8,0,4,0},
    {8,1,8,4,8,3,8,4,8,0,8,0,8,3,8,0,8,0,8,0,8,8,8,4},
    {8,0,8,0,8,3,8,4,8,1,8,0,3,0,8,0,8,1,8,0,8,0,4,0},
    {0,0,2,0,8,3,8,4,8,1,8,0,8,1,8,0,8,0,2,0,8,3,4,0},
    {8,1,8,2,3,0,4,0,8,0,8,0,8,0,8,0,1,0,8,2,3,0,8,4},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {8,0,2,0,8,3,8,4,8,0,8,0,8,3,8,0,8,1,8,0,8,3,4,0},
    {8,3,8,2,8,3,8,4,8,0,8,0,3,0,8,0,8,8,8,2,8,0,8,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,1,8,2,8,3,8,4,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0},
    {8,0,2,0,3,0,4,0,1,0,1,0,8,0,8,0,1,0,8,0,0,0,8,0},
    {8,0,8,0,8,3,8,4,1,0,8,0,8,3,8,0,8,0,2,0,8,3,8,0},
    {1,0,8,0,8,3,8,4,8,0,8,0,3,0,8,0,8,1,2,0,8,3,8,0},
    {1,0,8,0,3,0,4,0,8,1,8,1,8,0,8,0,0,0,8,0,3,0,8,0},
    {8,0,8,0,3,0,4,0,8,0,8,0,8,0,8,0,1,0,8,0,3,0,8,0},
    {8,0,8,8,8,0,8,4,8,0,8,2,8,0,8,0,8,1,8,2,8,4,8,4},
    {8,1,8,0,8,0,4,0,8,1,2,0,8,0,8,0,8,0,8,2,8,0,8,4},
    {8,1,8,1,8,0,4,0,0,0,2,0,8,3,8,0,1,0,8,0,8,0,8,4},
    {8,0,8,8,8,0,8,4,8,1,8,2,8,3,8,0,8,0,8,0,8,4,8,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {8,1,8,2,8,0,4,0,8,0,8,2,8,0,8,0,8,0,8,2,3,0,8,4},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {8,0,8,2,8,0,4,0,8,0,8,2,8,3,8,0,8,0,8,0,8,3,8,4},
    {8,0,8,2,8,0,8,4,8,0,8,2,3,0,8,0,1,0,8,2,8,0,8,0},
    {8,1,2,0,8,0,4,0,1,0,2,0,8,3,8,0,8,0,2,0,8,0,8,0},
    {1,0,2,0,8,0,4,0,2,0,2,0,8,0,8,0,8,1,8,0,8,8,8,0},
    {8,0,8,2,8,0,8,4,8,1,8,2,8,0,8,0,8,0,8,0,8,0,8,0},
    {8,0,8,0,8,0,4,0,8,1,2,0,8,3,8,0,1,0,2,0,8,3,8,0},
    {8,1,8,0,8,0,4,0,8,0,2,0,8,3,8,0,8,0,2,0,8,3,8,0},
    {1,0,8,0,8,0,4,0,1,0,2,0,8,0,8,0,1,0,8,0,8,3,8,0},
    {8,0,8,0,8,0,4,0,8,0,8,2,8,0,8,0,8,0,8,0,3,0,8,0},
    {8,0,8,0,8,0,8,0,8,8,8,2,8,8,8,4,8,3,8,4,8,3,8,4},
    {1,0,1,0,8,0,8,0,1,0,8,2,0,0,8,4,8,1,8,0,3,0,4,0},
    {8,1,8,1,8,0,8,0,0,0,8,2,3,0,8,4,8,0,8,2,3,0,4,0},
    {8,0,8,0,8,0,8,0,8,1,2,0,8,3,4,0,1,0,2,0,8,3,8,4},
    {0,0,2,0,8,0,8,0,8,1,8,2,8,1,8,4,8,0,8,0,8,0,4,0},
    {8,1,2,0,8,0,8,0,8,0,8,2,8,0,8,4,8,1,8,0,8,0,4,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0},
    {8,0,8,2,8,0,8,0,8,0,8,2,3,0,8,4,8,1,8,2,8,0,4,0},
    {8,8,8,2,8,0,8,0,8,3,8,2,8,3,8,4,8,0,8,0,3,0,8,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {1,0,8,2,8,0,8,0,8,0,8,2,8,0,8,4,8,0,8,2,3,0,8,0},
    {8,0,2,0,8,0,8,0,1,0,8,2,8,0,8,4,8,1,8,2,3,0,8,0},
    {8,0,8,0,8,0,8,0,8,1,8,2,8,3,8,4,8,0,8,0,8,0,8,0},
    {1,0,8,0,0,0,8,0,8,0,2,0,3,0,4,0,1,0,1,0,8,0,8,0},
    {8,1,8,0,8,8,8,0,1,0,2,0,8,0,4,0,2,0,2,0,8,0,8,0},
    {8,0,8,0,8,0,8,0,8,0,2,0,8,0,4,0,1,0,2,0,8,0,8,0},
    {0,0,8,0,3,0,8,0,8,0,8,0,8,0,4,0,8,1,8,1,8,3,8,4},
    {1,0,8,0,3,0,8,0,8,1,8,0,8,0,4,0,8,0,8,0,8,3,4,0},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {8,0,8,0,8,3,8,0,1,0,8,0,3,0,8,4,8,0,8,2,3,0,4,0},
    {8,0,8,2,3,0,8,0,8,1,8,0,8,8,4,0,1,0,8,4,8,0,8,4},
    {8,1,8,2,8,3,8,0,8,0,8,0,8,4,8,4,8,0,8,8,8,0,8,4},
    {8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {8,0,8,2,8,3,8,0,8,0,8,0,8,3,4,0,8,0,8,2,8,0,8,4},
    {8,0,2,0,8,3,8,0,8,0,8,0,3,0,8,4,8,1,8,0,8,3,8,0},
    {8,1,2,0,8,3,8,0,1,0,8,0,8,3,8,4,8,0,8,0,3,0,8,0},
    {8,1,8,2,3,0,8,0,8,0,8,0,8,0,4,0,1,0,8,2,3,0,8,0},
    {8,0,2,0,8,3,8,0,8,1,8,0,8,0,4,0,8,0,2,0,8,3,8,0},
    {0,0,8,0,3,0,8,0,1,0,8,0,3,0,4,0,8,1,8,1,8,0,8,0},
    {8,1,8,0,8,3,8,0,8,0,8,0,8,3,8,4,8,0,8,0,8,0,8,0},
    {1,0,8,0,8,3,8,0,1,0,8,0,8,0,4,0,8,1,8,2,8,0,8,0},
    {8,0,8,0,8,3,8,0,8,0,8,0,8,0,4,0,8,0,2,0,8,0,8,0},
    {8,0,8,0,3,0,8,0,8,8,8,2,8,0,8,0,8,3,8,2,8,3,8,4},
    {8,8,8,8,8,8,8,8,0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8},
    {1,0,8,0,8,3,8,0,8,0,2,0,8,3,8,0,8,0,8,0,8,3,8,4},
    {8,0,8,0,3,0,8,0,1,0,8,2,3,0,8,0,8,1,8,0,3,0,4,0},
    {8,0,8,2,3,0,8,0,8,1,8,2,8,0,8,0,8,0,2,0,8,0,8,4},
    {1,0,2,0,8,3,8,0,8,0,2,0,8,0,8,0,8,1,2,0,8,0,4,0},
    {8,1,2,0,8,3,8,0,1,0,2,0,8,3,8,0,8,0,8,0,8,0,4,0},
    {8,0,8,2,3,0,8,0,8,0,2,0,8,3,8,0,1,0,8,0,8,0,8,4},
    {8,0,2,0,8,3,8,0,8,8,2,0,8,3,8,0,8,3,2,0,8,3,8,0},
    {0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8},
    {1,0,2,0,3,0,8,0,8,2,8,2,8,0,8,0,8,8,8,0,8,3,8,0},
    {8,0,2,0,3,0,8,0,8,1,8,2,8,0,8,0,8,1,8,0,3,0,8,0},
    {8,8,8,0,8,3,8,0,1,0,2,0,3,0,8,0,8,2,8,2,8,0,8,0},
    {8,1,8,0,3,0,8,0,8,0,2,0,3,0,8,0,8,1,8,2,8,0,8,0},
    {1,0,8,0,3,0,8,0,8,1,8,2,8,0,8,0,8,0,8,0,8,0,8,0},
    {8,0,8,0,3,0,8,0,8,0,2,0,8,0,8,0,1,0,8,0,8,0,8,0},
    {8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,1,8,2,8,3,8,4},
    {1,0,0,0,8,0,8,0,1,0,8,0,1,0,8,0,8,0,2,0,3,0,4,0},
    {8,1,8,1,8,0,8,0,0,0,8,0,3,0,8,0,1,0,8,0,3,0,4,0},
    {8,0,8,0,8,0,8,0,8,1,8,0,8,3,8,0,8,0,8,0,8,3,8,4},
    {2,0,2,0,8,0,8,0,8,1,8,0,8,8,8,0,1,0,2,0,8,0,4,0},
    {8,1,8,2,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,2,8,0,8,4},
    {1,0,2,0,8,0,8,0,1,0,8,0,8,3,8,0,1,0,8,0,8,0,4,0},
    {8,0,8,2,8,0,8,0,8,0,8,0,8,3,8,0,8,0,8,0,8,0,8,4},
    {8,8,8,2,8,0,8,0,8,3,8,0,8,3,8,0,1,0,2,0,3,0,8,0},
    {8,1,2,0,8,0,8,0,8,1,8,0,8,3,8,0,8,0,2,0,3,0,8,0},
    {1,0,2,0,8,0,8,0,8,0,8,0,8,0,8,0,8,1,8,0,8,3,8,0},
    {8,0,2,0,8,0,8,0,8,1,8,0,8,0,8,0,8,0,8,0,8,3,8,0},
    {8,0,8,0,8,0,8,0,1,0,8,0,3,0,8,0,8,1,8,2,8,0,8,0},
    {8,1,8,0,8,0,8,0,8,0,8,0,3,0,8,0,8,0,8,2,8,0,8,0},
    {1,0,8,0,8,0,8,0,1,0,8,0,8,0,8,0,8,1,8,0,8,0,8,0},
    {8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0,8,0}
};

static int edgeTable[256] = {
    0x0, 0x111, 0x241, 0x350, 0x412, 0x503, 0x653, 0x742, 
    0x842, 0x953, 0xa03, 0xb12, 0xc50, 0xd41, 0xe11, 0xf00, 
    0x124, 0x35, 0x365, 0x274, 0x536, 0x427, 0x777, 0x666, 
    0x966, 0x877, 0xb27, 0xa36, 0xd74, 0xc65, 0xf35, 0xe24, 
    0x284, 0x395, 0xc5, 0x1d4, 0x696, 0x787, 0x4d7, 0x5c6, 
    0xac6, 0xbd7, 0x887, 0x996, 0xed4, 0xfc5, 0xc95, 0xd84, 
    0x3a0, 0x2b1, 0x1e1, 0xf0, 0x7b2, 0x6a3, 0x5f3, 0x4e2, 
    0xbe2, 0xaf3, 0x9a3, 0x8b2, 0xff0, 0xee1, 0xdb1, 0xca0, 
    0x428, 0x539, 0x669, 0x778, 0x3a, 0x12b, 0x27b, 0x36a, 
    0xc6a, 0xd7b, 0xe2b, 0xf3a, 0x878, 0x969, 0xa39, 0xb28, 
    0x50c, 0x41d, 0x74d, 0x65c, 0x11e, 0xf, 0x35f, 0x24e, 
    0xd4e, 0xc5f, 0xf0f, 0xe1e, 0x95c, 0x84d, 0xb1d, 0xa0c, 
    0x6ac, 0x7bd, 0x4ed, 0x5fc, 0x2be, 0x3af, 0xff, 0x1ee, 
    0xeee, 0xfff, 0xcaf, 0xdbe, 0xafc, 0xbed, 0x8bd, 0x9ac, 
    0x788, 0x699, 0x5c9, 0x4d8, 0x39a, 0x28b, 0x1db, 0xca, 
    0xfca, 0xedb, 0xd8b, 0xc9a, 0xbd8, 0xac9, 0x999, 0x888, 
    0x888, 0x999, 0xac9, 0xbd8, 0xc9a, 0xd8b, 0xedb, 0xfca, 
    0xca, 0x1db, 0x28b, 0x39a, 0x4d8, 0x5c9, 0x699, 0x788, 
    0x9ac, 0x8bd, 0xbed, 0xafc, 0xdbe, 0xcaf, 0xfff, 0xeee, 
    0x1ee, 0xff, 0x3af, 0x2be, 0x5fc, 0x4ed, 0x7bd, 0x6ac, 
    0xa0c, 0xb1d, 0x84d, 0x95c, 0xe1e, 0xf0f, 0xc5f, 0xd4e, 
    0x24e, 0x35f, 0xf, 0x11e, 0x65c, 0x74d, 0x41d, 0x50c, 
    0xb28, 0xa39, 0x969, 0x878, 0xf3a, 0xe2b, 0xd7b, 0xc6a, 
    0x36a, 0x27b, 0x12b, 0x3a, 0x778, 0x669, 0x539, 0x428, 
    0xca0, 0xdb1, 0xee1, 0xff0, 0x8b2, 0x9a3, 0xaf3, 0xbe2, 
    0x4e2, 0x5f3, 0x6a3, 0x7b2, 0xf0, 0x1e1, 0x2b1, 0x3a0, 
    0xd84, 0xc95, 0xfc5, 0xed4, 0x996, 0x887, 0xbd7, 0xac6, 
    0x5c6, 0x4d7, 0x787, 0x696, 0x1d4, 0xc5, 0x395, 0x284, 
    0xe24, 0xf35, 0xc65, 0xd74, 0xa36, 0xb27, 0x877, 0x966, 
    0x666, 0x777, 0x427, 0x536, 0x274, 0x365, 0x35, 0x124, 
    0xf00, 0xe11, 0xd41, 0xc50, 0xb12, 0xa03, 0x953, 0x842, 
    0x742, 0x653, 0x503, 0x412, 0x350, 0x241, 0x111, 0x0
};

struct iso
{
    block3 aabb;
    vec scale;
    virtual double density(vec p){ return 0; };
};

struct iso_point
{
    double d;
    vec p;
    iso_point(){};
    iso_point(vec point){p = point;}
    iso_point(double dens){d = dens;}
    iso_point(double dens,vec point){ d = dens; p = point; }
};

struct march_cube
{
    iso_point corners[8];
    unsigned int cubeindex;
    cube* c;
    vec o;
    int r,st;
    iso* ig;
    uint cc[12];
    
    march_cube(){};
    march_cube(cube &cu,int x,int y,int z, int rgrid, iso* iso_geom, int step){ march_cube(cu,vec(x*rgrid,y*rgrid,z*rgrid), rgrid, iso_geom, step); }
    march_cube(cube &cu,vec origin, int rgrid, iso* iso_geom, int step)
    {
        c = &cu;
        ig = iso_geom;
        r = rgrid;
        st = (r==1)?0:step;
        o = origin;
        init_corners();
        init_cube_index();
        if(devideable()){
            devide();
        }else{
            render();
        }
    }
    march_cube(cube &cu,unsigned int ci)
    {
        c = &cu;
        cubeindex = ci;
        for(int i=0;i<24;i+=2){
            if(bendTable[cubeindex][i]==8 || bendTable[cubeindex][i]==0){
                c->edges[int(i/2)] = 0x10*bendTable[cubeindex][i];
            }else{
                c->edges[int(i/2)] = 0x10*3;
            }
            if(bendTable[cubeindex][i+1]==8 || bendTable[cubeindex][i+1]==0){
                c->edges[int(i/2)] += 0x01*bendTable[cubeindex][i+1];
            }else{
                c->edges[int(i/2)] += 0x01*3;
            }
        }
    }
    march_cube(cube &cu)
    {
        c = &cu;
    }
    void init_corners()
    {
        corners[0] = iso_point(o);
        corners[1] = iso_point(vec(r,0,0).add(o));
        corners[2] = iso_point(vec(0,r,0).add(o));
        corners[3] = iso_point(vec(r,r,0).add(o));
        corners[4] = iso_point(vec(0,0,r).add(o));
        corners[5] = iso_point(vec(r,0,r).add(o));
        corners[6] = iso_point(vec(0,r,r).add(o));
        corners[7] = iso_point(vec(r,r,r).add(o));
        loopi(8){ corners[i].d = ig->density(corners[i].p); }
    }
    void init_cube_index()
    {
        cubeindex = 0;
        if(corners[0].d < 0) cubeindex |= 1;
        if(corners[1].d < 0) cubeindex |= 2;
        if(corners[2].d < 0) cubeindex |= 4;
        if(corners[3].d < 0) cubeindex |= 8;
        if(corners[4].d < 0) cubeindex |= 16;
        if(corners[5].d < 0) cubeindex |= 32;
        if(corners[6].d < 0) cubeindex |= 64;
        if(corners[7].d < 0) cubeindex |= 128;
    }
    void init_cube_cut()
    {
        if(edgeTable[cubeindex] & 1) cc[0] = calc_bendvalue(corners[0],corners[1]);
        if(edgeTable[cubeindex] & 2) cc[1] = calc_bendvalue(corners[2],corners[3]);
        if(edgeTable[cubeindex] & 4) cc[2] = calc_bendvalue(corners[4],corners[5]);
        if(edgeTable[cubeindex] & 8) cc[3] = calc_bendvalue(corners[6],corners[7]);
        if(edgeTable[cubeindex] & 16) cc[4] = calc_bendvalue(corners[0],corners[2]);
        if(edgeTable[cubeindex] & 32) cc[5] = calc_bendvalue(corners[4],corners[6]);
        if(edgeTable[cubeindex] & 64) cc[6] = calc_bendvalue(corners[1],corners[3]);
        if(edgeTable[cubeindex] & 128) cc[7] = calc_bendvalue(corners[5],corners[7]);
        if(edgeTable[cubeindex] & 256) cc[8] = calc_bendvalue(corners[0],corners[4]);
        if(edgeTable[cubeindex] & 512) cc[9] = calc_bendvalue(corners[1],corners[5]);
        if(edgeTable[cubeindex] & 1024) cc[10] = calc_bendvalue(corners[2],corners[6]);
        if(edgeTable[cubeindex] & 2048) cc[11] = calc_bendvalue(corners[3],corners[7]);
    }
    void devide()
    {
        int rh = r/2;
        int ns = st-1;
        c->children = newcubes();
        loopi(8){
            ivec v(i, o.x, o.y, o.z, rh);
            march_cube(c->children[i], v.tovec(), rh, ig, ns);
        }
    }
    void render()
    {
        if(cube_in_cut()){
            if(cube_in()){ solidfaces(*c); }
            else
            {
                solidfaces(*c);
                init_cube_cut();
                for(int i=0;i<24;i+=2){
                    if(bendTable[cubeindex][i]==8 || bendTable[cubeindex][i]==0){
                        c->edges[int(i/2)] = 0x10*bendTable[cubeindex][i];
                    }else{
                        c->edges[int(i/2)] = 0x10*cc[int(i/8)*4+(bendTable[cubeindex][i]-1)];
                    }
                    if(bendTable[cubeindex][i+1]==8 || bendTable[cubeindex][i+1]==0){
                        c->edges[int(i/2)] += 0x01*bendTable[cubeindex][i+1];
                    }else{
                        c->edges[int(i/2)] += 0x01*cc[int(i/8)*4+(bendTable[cubeindex][i+1]-1)];
                    }
                }
            }
        }else{
            emptyfaces(*c);
        }
    }
    unsigned int calc_bendvalue(iso_point p1, iso_point p2)
    {
        if(fabs(p1.d) < 0.00001) return 0;
        if(fabs(p2.d) < 0.00001) return 8;
        if(fabs(p1.d-p2.d) < 0.00001) return 0;
        double mu = -p1.d / (p2.d-p1.d);
        return (unsigned int)((mu*8)+0.5);
    }
    bool devideable()
    {
        return (st>0 && !cube_out() && !cube_in());
    }

    bool cube_in_cut()
    {
        if(cubeindex==0) return false;
        return true;
    }
    bool cube_in()
    {
        if(cubeindex==255) return true;
        return false;
    }
    bool cube_out()
    {
        if(cubeindex==0) return true;
        return false;
    }
};

void cubecreate(int x, int y, int z, int grid, uint *face, int *tex)
{
    cube *c;
    c = &lookupcube(x, y, z, grid);
    if(c->children) discardchildren(*c, true);
    loopi(3) c->faces[i] = face[i];
    loopi(6) c->texture[i] = tex[i];
}

#define DOT(x,y,z,g) (x*g[0] + y*g[1] + z*g[2])

#define FASTFLOOR(x) (x > 0 ? (int)x : (int)x - 1)

#define FORVOXEL(w,h,d) \
   for(int x = 0; x < w; x++) {\
   for(int y = 0; y < h; y++) {\
   for(int z = 0; z < d; z++) {

#define ENDFOR }}}

#define MAKECUBE(x,y,z,grid) \
   uint face[3]; int tex[6];\
   loopi(3) face[i]=uint(double(2155905152ULL));\
   loopi(6) tex[i]= 1;\
   cubecreate(x*grid, y*grid, z*grid, grid, face, tex);

double grad[12][3] = {
    {1.0,1.0,0.0},{-1.0,1.0,0.0},{1.0,-1.0,0.0},{-1.0,-1.0,0.0},
    {1.0,0.0,1.0},{-1.0,0.0,1.0},{1.0,0.0,-1.0},{-1.0,0.0,-1.0},
    {0.0,1.0,1.0},{0.0,-1.0,1.0},{0.0,1.0,-1.0},{0.0,-1.0,-1.0}
};

int perm[512] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};

double noise(double xin, double yin, double zin)
{
    double n0, n1, n2, n3;
    double F3 = 1.0/3.0;
    double s = (xin+yin+zin)*F3;
    int i = FASTFLOOR(xin+s);
    int j = FASTFLOOR(yin+s);
    int k = FASTFLOOR(zin+s);
    double G3 = 1.0/6.0;
    double t = (i+j+k)*G3;
    double X0 = i-t;
    double Y0 = j-t;
    double Z0 = k-t;
    double x0 = xin-X0;
    double y0 = yin-Y0;
    double z0 = zin-Z0;

    int i1, j1, k1, i2, j2, k2;

    if(x0 >= y0){
        if(y0 >= z0){
            i1=1; j1=0; k1=0; i2=1; j2=1; k2=0;
        }
        else if(x0 >= z0){
             i1=1; j1=0; k1=0; i2=1; j2=0; k2=1;
        }
        else{
            i1=0; j1=0; k1=1; i2=1; j2=0; k2=1;
        }
    }
    else{
        if(y0 < z0){
            i1=0; j1=0; k1=1; i2=0; j2=1; k2=1;
        }
        else if(x0 < z0){
            i1=0; j1=1; k1=0; i2=0; j2=1; k2=1;
        }
        else{
            i1=0; j1=1; k1=0; i2=1; j2=1; k2=0;
        }
    }

    double x1 = x0 - i1 + G3;
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0*G3;
    double y2 = y0 - j2 + 2.0*G3;
    double z2 = z0 - k2 + 2.0*G3;
    double x3 = x0 - 1.0 + 3.0*G3;
    double y3 = y0 - 1.0 + 3.0*G3;
    double z3 = z0 - 1.0 + 3.0*G3;

    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;

    int gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
    int gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
    int gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
    int gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;

    double t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if(t0 < 0)
    {
         n0 = 0.0;
    }
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * DOT(x0, y0, z0, grad[gi0]);
    }

    double t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if(t1 < 0)
    {
         n1 = 0.0;
    }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * DOT(x1, y1, z1, grad[gi1]);
    }

    double t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if(t2 < 0)
    {
         n2 = 0.0;
    }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * DOT(x2, y2, z2, grad[gi2]);
    }

    double t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if(t3 < 0)
    {
         n3 = 0.0;
    }
    else
    {
        t3 *= t3;
        n3 = t3 * t3 * DOT(x3, y3, z3, grad[gi3]);
    }
    return 16.0*(n0 + n1 + n2 + n3) + .5;
}

double simplex_noise(int octaves, double x, double y, double z)
{
    double value = 0.0;
    for(int i = 0; i<octaves; i++)
    {
        value += noise(
            x*pow(2, i),
            y*pow(2, i),
            z*pow(2, i)
        );
    }
    return value;
}

struct cylinder_iso:iso
{
    float r;
    float rr;
    float h;
    float rh;
    vec o;

    cylinder_iso(selinfo &sel, float rad, float height, vec off)
    {
        r = rad;
        rr = r*sel.grid;
        h = height;
        rh = h*sel.grid;
        sel.s = vec(ceilf(r*2),ceilf(r*2),ceilf(h)).add(vec(ceilf(off.x),ceilf(off.y),ceilf(off.z)));
        o = vec(rr).add(off.mul(vec(sel.grid)));
        aabb = block3(sel);
    }

    double density(vec p)
    {
        vec rp = p.sub(o);
        return double(rp.x*rp.x + rp.y*rp.y) - double(rr*rr);
    }
};

void buildcylinder(float *r, float *h, int *d, float *ox, float *oy, float *oz)
{
    cylinder_iso cyl = cylinder_iso(sel, *r, *h, vec(*ox, *oy, *oz));
    iso* iso_geom = &cyl;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));
    changed(sel);
}
COMMAND(buildcylinder, "ffifff");

int start_mcz = 0; 
double mcz = 0;

struct helix_iso:iso
{
    float r,rr;
    float br,rbr;
    vec o;
    helix_iso(selinfo &sel, float xr, float ir)
    {
        r = ir;
        br = xr;
        rr = r*sel.grid;
        rbr = br*sel.grid;
        sel.s = vec(ceilf((r+br)*2));
        o = vec(rr+rbr);
        aabb = block3(sel);
        mcz = 0;
    }
    double density(vec p)
    {
        vec rp = p.sub(o);
        double z;
        if(rp.z >= rbr - rr && rp.y < 0 && rp. x< 0)
        { 
            z = rp.z - rbr*2 - (atan2(rp.y,rp.x)/PI)*rbr;
        }
        else if(rp.z <= -(rbr-rr) && rp.y >= 0 && rp.x <= 0)
        {
            z = rp.z + rbr*2 - (atan2(rp.y,rp.x)/PI)*rbr;
        }
        else
        {
            z = rp.z - (atan2(rp.y,rp.x)/PI)*rbr;
        }
        double d = sqrt(rp.x*rp.x+rp.y*rp.y)-rbr;
        return double(d*d + z*z) - double(rr*rr);
    }
};

void buildhelix(float *r, float *br, int *d)
{
    helix_iso helix = helix_iso(sel, *br, *r);
    iso* iso_geom = &helix;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));
    changed(sel);
}
COMMAND(buildhelix, "ffi");

VAR(mandelbrotgrid, 0, 8, INT_MAX);
VAR(mandelbrotwidth, 0, 300, INT_MAX);
VAR(mandelbrotheight, 0, 200, INT_MAX);
VAR(mandelbrotdepth, 0, 400, INT_MAX);
VAR(mandelbrotiterations, 0, 250, INT_MAX);

#define MaxX 1.0
#define MinX -2.0
#define MaxY 1.0
#define MinY -1.0
#define MaxZR 2.0
#define MinZR -2.0
#define xSize ((MaxX - MinX) / mandelbrotwidth)
#define ySize ((MaxY - MinY) / mandelbrotheight)
#define zrSize ((MaxZR - MinZR) / mandelbrotdepth)

void multiply(long double *dA, long double *dB, long double *dC, long double *dD)
{
    long double a = *dA, b = *dB;
    *dA = (a * (*dC)) - (b * (*dD));
    *dB = (b * (*dC)) + (a * (*dD));
}

void square(long double *dA, long double *dB)
{
    long double dA1 = *dA;
    long double dB1 = *dB;
    multiply(dA, dB, &dA1, &dB1);
}

void exponent(long double *dA, long double *dB, int power)
{
    assert(power > 0);
    if(power == 1) return;
    else
    {
        long double dA1 = *dA;
        long double dB1 = *dB;
        for(int i = 1; i < power; i++)
        {
            multiply(dA, dB, &dA1, &dB1);
        }
    }
}

bool iterator(long double cr, long double ci, long double zr, long double zi, int power)
{
    for(int count = 0; ((zr*zr) + (zi*zi) <= 4.0) && (count < mandelbrotiterations); count++)
    {
        exponent(&zr, &zi, power);
        zr += cr;
        zi += ci;
        count++;
    }
    if((zr*zr) + (zi*zi) <= 4.0) return 1;
    else return 0;
}

void buildmandelbrot()
{
    int power = 2, tex[6];
    uint face[3];

    for(int nzr = 0; nzr < mandelbrotdepth; nzr++)
    {
        long double zr = (nzr * zrSize) + MinZR;
        long double zi = 0.0;
        for(int y = 0; y < mandelbrotheight; y++)
        {
            for(int x = 0; x < mandelbrotwidth; x++)
            {
                long double cr = (x * xSize) + MinX;
                long double ci = (y * ySize) + MinY;
                if(iterator(cr, ci, zr, zi, power))
                {
                    loopi(3) face[i] = uint(double(2155905152ULL));
                    loopi(6) tex[i] = 1;               
                    cubecreate(x*mandelbrotgrid, nzr*mandelbrotgrid, y*mandelbrotgrid, mandelbrotgrid, face, tex);
                }
            }
        }
    }
    allchanged();
}
COMMAND(buildmandelbrot, "");

struct octahedron_iso:iso
{
    float r;
    float rr;
    vec o;

    octahedron_iso(selinfo &sel, float rad, vec off)
    {
        r = rad;
        rr = r*sel.grid;
        sel.s = vec(ceilf(r*2)).add(vec(ceilf(off.x),ceilf(off.y),ceilf(off.z)));
        o = vec(rr).add(off.mul(vec(sel.grid)));
        aabb = block3(sel);
    }

    double density(vec p)
    {
        vec rp = p.sub(o);
        return double(abs(rp.x)+abs(rp.y)+abs(rp.z)-rr);
    }
};

void buildoctahedron(float *r, int *d, float *ox, float *oy, float *oz)
{
    octahedron_iso oct = octahedron_iso(sel, *r, vec(*ox, *oy, *oz));
    iso* iso_geom = &oct;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));
    changed(sel);
};
COMMAND(buildoctahedron, "fifff");

struct paraboloid_iso:iso
{
    float r;
    float rr;
    vec o;

    paraboloid_iso(selinfo &sel, float rad, vec off)
    {
        r = rad;
        rr = r*sel.grid;
        sel.s = vec(ceilf(r*2)).add(vec(ceilf(off.x),ceilf(off.y),ceilf(off.z)));
        o = vec(rr).add(off.mul(vec(sel.grid)));
        aabb = block3(sel);
    }

    double density(vec p)
    {
        vec rp = p.sub(o);
        return double(rp.x*rp.x + rp.z + rp.y*rp.y - rr);
    }
};

void buildparaboloid(float *r, int *d, float *ox, float *oy, float *oz)
{
    paraboloid_iso para = paraboloid_iso(sel, *r, vec(*ox, *oy, *oz));
    iso* iso_geom = &para;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));
    changed(sel);
};
COMMAND(buildparaboloid, "fifff");

struct noise_iso:iso
{
   float r;
   float rr;
   vec o;

   noise_iso(selinfo &sel, float rad, vec off)
   {
      r = rad;
      rr = r*sel.grid;
      sel.s = vec(ceilf(r*2)).add(vec(ceilf(off.x),ceilf(off.y),ceilf(off.z)));
      o = vec(rr).add(off.mul(vec(sel.grid)));
      aabb = block3(sel);
   }
   double density(vec p)
   {
      vec rp = p.sub(o);
      return simplex_noise(1, rp.x*.01, rp.y*.01, rp.z*.01)-.5;
   }
};

void buildsimplexnoise(float *r, int *d, float *ox, float *oy, float *oz)
{
    noise_iso simplex = noise_iso(sel, *r, vec(*ox, *oy, *oz));
    iso* iso_geom = &simplex;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));
    changed(sel);

}
COMMAND(buildsimplexnoise, "fifff");

void buildsimplexsphere(int *innerr, int *outerr, int *grid, int *noise, float *scaling, float *threshold)
{
    int w, h, d;
    w = h = d = *outerr*2;
    int r2 = *outerr**outerr;
    int ir2 = *innerr**innerr;

    FORVOXEL(w, h, d)
    int dist2 = ((x-*outerr)*(x-*outerr) + (y-*outerr)*(y-*outerr) + (z-*outerr)*(z-*outerr));
    if((ir2 <= dist2) && (r2 > dist2))
    {
        if(*noise != 0)
        {
            double a = simplex_noise(1, x**scaling, y**scaling, z**scaling);
            if(a > *threshold) { MAKECUBE(x, y, z, *grid); }
         }
         else { MAKECUBE(x, y, z, *grid); }
    }
    ENDFOR
    allchanged();
}
COMMAND(buildsimplexsphere, "iiiiff");

struct sphere_iso:iso
{
    float r;
    float rr;
    vec o;

    sphere_iso(selinfo &sel, float rad, vec off, vec scl)
    {
        r = rad;
        rr = r*sel.grid;
        scale = scl;
        sel.s = vec(ceilf(r*2)).add(vec(ceilf(off.x),ceilf(off.y),ceilf(off.z)));
        o = vec(rr).add(off.mul(vec(sel.grid)));
        aabb = block3(sel);
    }

    double density(vec p)
    {
        vec rp = p.sub(o);
        return double((rp.x/scale.x)*(rp.x/scale.x) + (rp.y/scale.y)*(rp.y/scale.y) + (rp.z/scale.z)*(rp.z/scale.z)) - double(rr*rr);
    }
};

void buildsphere(float *r, int *d, float *ox, float *oy, float *oz, float *sx, float *sy, float *sz)
{
    sphere_iso sphere = sphere_iso(sel, *r, vec(*ox, *oy, *oz), vec(*sx, *sy, *sz));
    iso* iso_geom = &sphere;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));
    changed(sel);
};
COMMAND(buildsphere, "fiffffff");

void rotateCoords(vec *rp, vec rot)
{
   if(rot.x || rot.y || rot.z)
   {
      int x = rp->x;
      int y = rp->y;
      int z = rp->z;
      double phi = rot.x;
      double theta = rot.y;
      double psi = rot.z;
      rp->x = (cos(theta)*cos(psi))*x + (-cos(phi)*sin(psi)+sin(phi)*sin(theta)*cos(psi))*y +
            (sin(phi)*sin(psi)+cos(phi)*sin(theta)*cos(psi))*z;
      rp->y = (cos(theta)*sin(psi))*x + (cos(phi)*cos(psi)+sin(phi)*sin(theta)*sin(psi))*y + 
            (-sin(phi)*cos(psi)+cos(phi)*sin(theta)*sin(psi))*z;
      rp->z = (-sin(theta))*x + (sin(phi)*cos(theta))*y + (cos(phi)*cos(theta))*z;
   }
};

#define SQ(x) ((x)*(x))

struct torusm_iso:iso
{
    float r;
    float rr;
    float RR;
    double sign;
    vec o;

    torusm_iso(selinfo &sel, float rad, float R, vec scl)
    {
        sign = (rad < 0 ? -1 : 1);
        r = abs(rad);
        rr = r*sel.grid;
        RR = R*sel.grid;
        scale = scl;
        sel.s = vec(ceilf(r*2+R*2), ceilf(r*2+R*2), ceilf(r*2+R*2));
        o = vec(rr+RR);
        aabb = block3(sel);
    }

    double density(vec p)
    {
        vec rp = p.sub(o);
        #define F(x, y, z) (SQ(RR - sqrt(SQ(x) + SQ(y))) + SQ(z) - SQ(rr))
        rotateCoords(&rp, scale);
        return sign*double(min(F(rp.x, rp.y, rp.z), min(F(rp.y, rp.z, rp.x), F(rp.z, rp.x, rp.y))));
    }
};

void buildtorus(float *r, float *R, int *d, float *ox, float *oy, float *oz)
{
    torusm_iso tor = torusm_iso(sel, *r, *R, vec(*ox, *oy, *oz));
    iso* iso_geom = &tor;
    iso_geom->aabb.orient = 4;
    loopxyz(iso_geom->aabb, iso_geom->aabb.grid, march_cube(c, x, y, z, iso_geom->aabb.grid, iso_geom, *d));   
    changed(sel);
};
COMMAND(buildtorus, "ffifff");

void importsmc(char *name, bool *voxel)
{
    stream *f = openfile(name, "r");
    if(!f)
    {
        conoutf(CON_ERROR, "\f9FanEd\f7::importsmc: \f3#error: \f7could not load .smc file");
        return;
    }
    char buf[1024];
    double d1, d2, d3, d4, d5, d6, d7, d8;
    int x, y, z, g, cnt = 0;
    march_cube *mc;
    while(f->getline(buf, sizeof(buf)))
    {
        cnt++;
        sscanf(buf, "%i %i %i %i %lf %lf %lf %lf %lf %lf %lf %lf", &x,&y,&z,&g,&d1,&d2,&d3,&d4,&d5,&d6,&d7,&d8);
        if(*voxel)
        {
            solidfaces(lookupcube(sel.o.x+x, sel.o.y+y, sel.o.z+z, g));
        }
        else
        {
            mc = new march_cube(lookupcube(sel.o.x+x, sel.o.y+y, sel.o.z+z, g));
            mc->corners[0] = iso_point(d1);
            mc->corners[1] = iso_point(d2);
            mc->corners[2] = iso_point(d4);
            mc->corners[3] = iso_point(d3);
            mc->corners[4] = iso_point(d5);
            mc->corners[5] = iso_point(d6);
            mc->corners[6] = iso_point(d8);
            mc->corners[7] = iso_point(d7);
            mc->init_cube_index();
            mc->render();
        }
    }
    conoutf(CON_INFO, "\f9FanEd\f7::importsmc: \f9%i\f7 cubes imported", cnt);
    f->close();
    allchanged();
};
COMMAND(importsmc, "si");

void importrec(char *name)
{
    stream *f = openfile(name, "r");
    if(!f)
    {
        conoutf(CON_ERROR, "\f9FanEd\f7::importrec: \f3#error: \f7could not load .rec file");
        return;
    }

    char buf[512];
    char *p;
    int j = 0, k = 0;
    int linedata[16];
    int tex[6];
    uint face[3]; 

    while(f->getline(buf, sizeof(buf)))
    {
        j = 0;
        p = strtok(buf, " ");
        k++;

        while(p != NULL)
        {
            if(j <= 15) linedata[j] = atoi(p);
            p = strtok(NULL, " ");
            j++;
        }

        loopi(3) face[i] = uint(double(linedata[5+i]+2147483647));
        loopi(6) tex[i] = linedata[8+i];
        if(linedata[0] == 1) cubecreate(linedata[1], linedata[2], linedata[3], linedata[4], face, tex);
    }
    conoutf(CON_INFO, "\f9FanEd\f7::importrec: \f9%d\f7 cubes imported", k);
    f->close();
    allchanged();
}
COMMAND(importrec, "s");

// Maze Generator by AC
int listpos(int *p, int elt);
void add2list(int *list, int elt);
void addneighborwalls(int cell, int m, int n, int *p);
void del2list(int *p, int ind);
void getsel(int *p);
void maze2list(int m, int n, int *p, int *list);
void mazecreate(int x, int y, int z, int dx, int dy, int dz, int grid, int face);
void primstep(int m, int n, int *walllist, int *cells, int *passwalls);
void printmaze(int m, int n, int *p);

int listpos(int *p, int elt)
{
    int i;
    for(i=1; i<=(*p); i++)
    {
        if(*(p+i) == elt) return i;
    }
    return -1;
}

void add2list(int *p, int elt)
{
    *(p+((*p)+1))=elt;
    *p=(*p)+1;
}

void addneighborwalls(int cell, int m, int n, int *p)
{
    div_t divr;
    divr = div(cell, n);

    if((cell<m*n-n+1)) {*(p + ((*p)+1)) = cell; *p = (*p)+1;}
    if((cell>n)) {*(p + ((*p)+1)) = cell-n; *p = (*p)+1;}
    if((divr.rem!=1)) {(*(p+ (*p)+1) = cell + m*n);*p = (*p)+1;}
    if((divr.rem!=0)) {(*(p+ (*p)+1) = cell + m*n+1);*p = (*p)+1;}
}

void del2list(int *p, int ind)
{
    if((ind>(*p)) || (ind<=0) || (*p == 0)) return;

    int i;

    for(i=ind; i<(*p); i++) {*(p+i)=*(p+i+1);}
    *p=(*p)-1;
}

void getsel(int *p)
{
    *(p+0)= sel.o.x;*(p+1)= sel.o.y;*(p+2)= sel.o.z;
    *(p+3)= sel.s.x;*(p+4)= sel.s.y;*(p+5)= sel.s.z;
    *(p+6)= sel.grid; *(p+7) = sel.orient;
}

void maze2list(int m, int n, int *p, int *list)
{
    int i, j, k=0;

    for(j=1; j<=n; j++)
    {
        *(list+k)=1;
        *(list+k+1)=1;
        k=k+2;
    }
    *(list+k)=1;

    k++;

    for(i = 1; i<=m; i++)
    {
        for(j=1; j<=n; j++) {if(listpos(p, n*(i-1)+j+m*n)>0) {k=k+2;} else {*(list+k)=1; k=k+2;}}
        *(list+k)=1;
        k++;
        for(j=1; j<=n; j++) {if(listpos(p, n*(i-1)+j)>0) {*(list+k)=1;k=k+2;} else {*(list+k)=1;*(list+k+1)=1;k=k+2;}}
        *(list+k)=1;
        k++;
    }
}

void mazecreate(int x, int y, int z, int dx, int dy, int dz, int grid, int face)
{
    int dir, mode;
    selinfo sel;
    sel.o.x = x; sel.o.y = y; sel.o.z = z;
    sel.s.x = dx; sel.s.y = dy; sel.s.z = dz;
    sel.grid = grid; sel.orient = face;
    sel.cx = 0; sel.cxs = 2; sel.cy = 0; sel.cys = 2;
    sel.corner = 0; dir = -1; mode = 1;
    mpeditface(dir, mode, sel, 1);
}

void primstep(int m, int n, int *walllist, int *cells, int *passwalls)
{
    if(walllist[0]!=0) {} else return;
    int wall;
    wall=walllist[(rand () % (walllist[0]))+1];
    int cell1, cell2;
    if(wall>m*n)
    {
        cell1=wall-m*n;
        cell2=cell1-1;
    }
    else
    {
        cell1=wall;
        cell2=wall+n;
    }

    if(*(cells+cell1)==0)
    {
        *(cells+cell1)=1;
        *(passwalls+((*passwalls)+1)) = wall;
        *passwalls = (*passwalls)+1;
        addneighborwalls(cell1, m, n, walllist);
    }

    if(*(cells+cell2)==0)
    {
        *(cells+cell2)=1;
        *(passwalls+((*passwalls)+1)) = wall;
        *passwalls = (*passwalls)+1;
        addneighborwalls(cell2, m, n, walllist);
    }
    del2list(walllist, listpos(walllist, wall));
}

void genmaze(int m, int n, int seed)
{
    if(!m) return;

    int x0, y0, z0, grid;

    int p[8];
    getsel(p); x0 = *(p+0);

    y0 = *(p+1);
    z0 = *(p+2);
    grid = *(p+6);

    int st=0; int i;
    int walllist[2*m*n+1]; int passwalls[2*m*n+1]; int cells[m*n+1];

    st = 0;
    srand(seed);
    for(i=1; i<=m*n; i++) { cells[i+1]=0; }
    passwalls[0]=0;
    i=(rand() % (m*n))+1;
    cells[i] = 1;
    walllist[0]=0;
    addneighborwalls(i, m, n, walllist);

    while (walllist[0]>0)
    {
        st++;
        primstep(m, n, walllist, cells, passwalls);
    }

    int list[(2*m+1)*(2*n+1)]; int j;
    for(i=0; i<(2*m+1)*(2*n+1); i++) { *(list+i)=0; }
    maze2list(m, n, passwalls, list);

    int k = 0;

    for(i=0; i<2*m+1; i++)
    {
        for(j=0; j<2*n+1; j++)
        {
            if(*(list+(2*n+1)*i+j)==1)
            {
                k++;
                mazecreate(i*grid+x0,j*grid+y0,0+z0,1,1,1,grid,5);}
        }
    }
    conoutf(CON_INFO, "\f9FanEd\f7::genmaze: \f9%d\f7 cubes generated", k);
}

ICOMMAND(genmaze, "iii", (int *m, int *n, int *seed), genmaze(*m, *n, *seed));

// End: Fanatic Edition

