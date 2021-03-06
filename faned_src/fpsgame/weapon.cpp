// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"

// Start: Fanatic Edition
VARP(blood, 0, 1, 1);
HVARP(bloodcolor, 0, 0x9F0000, 0xFFFFFF);
FVARP(bloodintensity, 0, 5.0f, INT_MAX);

HVARP(sgshotcolor, 0, 0xFF6600, 0xFFFFFF);
VARP(sgshotduration, 1, 100, 10000);
FVARP(sgshotsize, 0, 0.24f, 50.0f);

HVARP(cgshotcolor, 0, 0xFF6600, 0xFFFFFF);
VARP(cgshotduration, 1, 250, 10000);
FVARP(cgshotsize, 0, 0.24f, 50.0f);

HVARP(pishotcolor, 0, 0xFF6600, 0xFFFFFF);
VARP(pishotduration, 1, 250, 10000);
FVARP(pishotsize, 0, 0.24f, 50.0f);

HVARP(riflarecolor, 0, 0xFF6600, 0xFFFFFF);
VARP(riflareduration, 1, 250, 10000);
FVARP(riflaresize, 0, 0.24f, 50.0f);

VARP(rishotduration, 1, 500, 10000);
VARP(rishotgravity, -50, 40, 50);
VARP(rishotparticle, 0, 3, 4);
FVARP(rishotsize, 0, 0.5f, 50.0f);

VARP(ritraillaser, 0, 0, 1);
VARP(ritraillightning, 0, 0, 1);
VARP(ritrailsmoke, 0, 1, 1);
VARP(ritrailspin, 0, 0, 1);

HVARP(lasercolor, 0, 0x222222, 0xFFFFFF);
VARP(lasercolorrainbow, 0, 0, 1);
VARP(lasercolorteam, 0, 0, 1);

HVARP(lightningcolor, 0, 0x222222, 0xFFFFFF);
VARP(lightningcolorrainbow, 0, 0, 1);
VARP(lightningcolorteam, 0, 0, 1);

HVARP(smokecolor, 0, 0x222222, 0xFFFFFF);
VARP(smokecolorrainbow, 0, 0, 1);
VARP(smokecolorteam, 0, 0, 1);

HVARP(teamcolorred, 0, 0xFC0907, 0xFFFFFF);
HVARP(teamcolorblue, 0, 0x0789FC, 0xFFFFFF);
HVARP(teamcolorgreen, 0, 0x32FF64, 0xFFFFFF);

HVARP(spincolor, 0, 0x222222, 0xFFFFFF);
VARP(spincolorrainbow, 0, 0, 1);
VARP(spincolorteam, 0, 0, 1);

VARP(shotsparks, 0, 1, 1);

VARP(ducky, 0, 0, 1);
// End: Fanatic Edition

namespace game
{
    static const int MONSTERDAMAGEFACTOR = 4;
    static const int OFFSETMILLIS = 500;
    vec rays[MAXRAYS];

    struct hitmsg
    {
        int target, lifesequence, info1, info2;
        ivec dir;
    };
    vector<hitmsg> hits;

    VARP(maxdebris, 10, 75, 1000); // Fanatic Edition
    VARP(maxbarreldebris, 5, 10, 1000);

    ICOMMAND(getweapon, "", (), intret(player1->gunselect));

    void gunselect(int gun, fpsent *d)
    {
        if(gun!=d->gunselect)
        {
            addmsg(N_GUNSELECT, "rci", d, gun);
            playsound(S_WEAPLOAD, &d->o);
        }
        d->gunselect = gun;
    }

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        dir = (dir < 0 ? NUMGUNS-1 : 1);
        int gun = player1->gunselect;
        loopi(NUMGUNS)
        {
            gun = (gun + dir)%NUMGUNS;
            if(force || player1->ammo[gun]) break;
        }
        if(gun != player1->gunselect) gunselect(gun, player1);
        else playsound(S_NOAMMO);
    }
    ICOMMAND(nextweapon, "ii", (int *dir, int *force), nextweapon(*dir, *force!=0));

    int getweapon(const char *name)
    {
        const char *abbrevs[] = { "FI", "SG", "CG", "RL", "RI", "GL", "PI" };
        if(isdigit(name[0])) return parseint(name);
        else loopi(sizeof(abbrevs)/sizeof(abbrevs[0])) if(!strcasecmp(abbrevs[i], name)) return i;
        return -1;
    }

    void setweapon(const char *name, bool force = false)
    {
        int gun = getweapon(name);
        if(player1->state!=CS_ALIVE || gun<GUN_FIST || gun>GUN_PISTOL) return;
        if(force || player1->ammo[gun]) gunselect(gun, player1);
        else playsound(S_NOAMMO);
    }
    ICOMMAND(setweapon, "si", (char *name, int *force), setweapon(name, *force!=0));

    void cycleweapon(int numguns, int *guns, bool force = false)
    {
        if(numguns<=0 || player1->state!=CS_ALIVE) return;
        int offset = 0;
        loopi(numguns) if(guns[i] == player1->gunselect) { offset = i+1; break; }
        loopi(numguns)
        {
            int gun = guns[(i+offset)%numguns];
            if(gun>=0 && gun<NUMGUNS && (force || player1->ammo[gun]))
            {
                gunselect(gun, player1);
                return;
            }
        }
        playsound(S_NOAMMO);
    }
    ICOMMAND(cycleweapon, "V", (tagval *args, int numargs),
    {
         int numguns = min(numargs, 7);
         int guns[7];
         loopi(numguns) guns[i] = getweapon(args[i].getstr());
         cycleweapon(numguns, guns);
    });

    void weaponswitch(fpsent *d)
    {
        if(d->state!=CS_ALIVE) return;
        int s = d->gunselect;
        if     (s!=GUN_CG     && d->ammo[GUN_CG])     s = GUN_CG;
        else if(s!=GUN_RL     && d->ammo[GUN_RL])     s = GUN_RL;
        else if(s!=GUN_SG     && d->ammo[GUN_SG])     s = GUN_SG;
        else if(s!=GUN_RIFLE  && d->ammo[GUN_RIFLE])  s = GUN_RIFLE;
        else if(s!=GUN_GL     && d->ammo[GUN_GL])     s = GUN_GL;
        else if(s!=GUN_PISTOL && d->ammo[GUN_PISTOL]) s = GUN_PISTOL;
        else                                          s = GUN_FIST;

        gunselect(s, d);
    }

    ICOMMAND(weapon, "V", (tagval *args, int numargs),
    {
        if(player1->state!=CS_ALIVE) return;
        loopi(7)
        {
            const char *name = i < numargs ? args[i].getstr() : "";
            if(name[0])
            {
                int gun = getweapon(name);
                if(gun >= GUN_FIST && gun <= GUN_PISTOL && gun != player1->gunselect && player1->ammo[gun]) { gunselect(gun, player1); return; }
            } else { weaponswitch(player1); return; }
        }
        playsound(S_NOAMMO);
    });

    void offsetray(const vec &from, const vec &to, int spread, float range, vec &dest)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);
        offset.mul((to.dist(from)/1024)*spread);
        offset.z /= 2;
        dest = vec(offset).add(to);
        vec dir = vec(dest).sub(from).normalize();
        raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
    }

    void createrays(int gun, const vec &from, const vec &to)
    {
        loopi(guns[gun].rays) offsetray(from, to, guns[gun].spread, guns[gun].range, rays[i]);
    }

    enum { BNC_GRENADE, BNC_GIBS, BNC_DEBRIS, BNC_BARRELDEBRIS };

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
        bool local;
        fpsent *owner;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;
        entitylight light;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };

    vector<bouncer *> bouncers;

    vec hudgunorigin(int gun, const vec &from, const vec &to, fpsent *d);

    void newbouncer(const vec &from, const vec &to, bool local, int id, fpsent *owner, int type, int lifetime, int speed, entitylight *light = NULL)
    {
        bouncer &bnc = *bouncers.add(new bouncer);
        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = type==BNC_DEBRIS ? 0.5f : 1.5f;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;
        if(light) bnc.light = *light;

        switch(type)
        {
            case BNC_GRENADE: bnc.collidetype = COLLIDE_ELLIPSE_PRECISE; break;
            case BNC_DEBRIS: case BNC_BARRELDEBRIS: bnc.variant = rnd(4); break;
            case BNC_GIBS: bnc.variant = rnd(3); break;
        }

        vec dir(to);
        dir.sub(from).normalize();
        bnc.vel = dir;
        bnc.vel.mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        if(type==BNC_GRENADE)
        {
            bnc.offset = hudgunorigin(GUN_GL, from, to, owner);
            if(owner==hudplayer() && !isthirdperson()) bnc.offset.sub(owner->o).rescale(16).add(owner->o);
        }
        else bnc.offset = from;
        bnc.offset.sub(bnc.o);
        bnc.offsetmillis = OFFSETMILLIS;

        bnc.resetinterp();
    }

    void bounced(physent *d, const vec &surface)
    {
        if(d->type != ENT_BOUNCE) return;
        bouncer *b = (bouncer *)d;
        if(b->bouncetype != BNC_GIBS || b->bounces >= 2) return;
        b->bounces++;
        // Start: Fanatic Edition
        int red = bloodcolor / 0x10000;
        int green = bloodcolor % 0x10000 / 0x100;
        int blue = bloodcolor % 0x100;
        adddecal(DECAL_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(~red, ~green, ~blue), rnd(4));
        // End: Fanatic Edition
    }

    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            
            // Start: Fanatic Edition
            int teamsmokecolor;
            if(!m_teammode) teamsmokecolor = teamcolorgreen;
            else
            {
                if(!joinred) teamsmokecolor = isteam(bnc.owner->team, player1->team) ? teamcolorblue : teamcolorred;
                else teamsmokecolor = isteam(bnc.owner->team, player1->team) ? teamcolorred : teamcolorblue;
            }
            // End: Fanatic Edition
            
            if(bnc.bouncetype==BNC_GRENADE && bnc.vel.magnitude() > 50.0f)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
                // Start: Fanatic Edition
                regular_particle_splash(PART_SMOKE, 1, 150, pos, smokecolorrainbow ? rnd(16777216) : (smokecolorteam ? teamsmokecolor : smokecolor), 2.4f, 50, -20);
                if(lookupmaterial(pos)==MAT_WATER) regular_particle_splash(PART_BUBBLE, 1, 500, pos, 0xFFFFFF, 1.0f, 25, 500);
                // End: Fanatic Edition
            }
            vec old(bnc.o);
            bool stopped = false;
            if(bnc.bouncetype==BNC_GRENADE) stopped = bounce(&bnc, 0.6f, 0.5f, 0.8f) || (bnc.lifetime -= time)<0;
            else
            {
                for(int rtime = time; rtime > 0;)
                {
                    int qtime = min(30, rtime);
                    rtime -= qtime;
                    if((bnc.lifetime -= qtime)<0 || bounce(&bnc, qtime/1000.0f, 0.6f, 0.5f, 1)) { stopped = true; break; }
                }
            }
            if(stopped)
            {
                if(bnc.bouncetype==BNC_GRENADE)
                {
                    int qdam = guns[GUN_GL].damage*(bnc.owner->quadmillis ? 4 : 1);
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, NULL, qdam, GUN_GL);
                    adddecal(DECAL_SCORCH, bnc.o, vec(0, 0, 1), guns[GUN_GL].exprad/2);
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, GUN_GL, bnc.id-maptime,
                                hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
                delete bouncers.remove(i--);
            }
            else
            {
                bnc.roll += old.sub(bnc.o).magnitude()/(4*RAD);
                bnc.offsetmillis = max(bnc.offsetmillis-time, 0);
            }
        }
    }

    void removebouncers(fpsent *owner)
    {
        loopv(bouncers) if(bouncers[i]->owner==owner) { delete bouncers[i]; bouncers.remove(i--); }
    }

    void clearbouncers() { bouncers.deletecontents(); }

    struct projectile
    {
        vec dir, o, to, offset;
        float speed;
        fpsent *owner;
        int gun;
        bool local;
        int offsetmillis;
        int id;
        entitylight light;
        // FIXME: int pc; // Fanatic Edition
    };
    vector<projectile> projs;

    void clearprojectiles() { projs.shrink(0); }

    void newprojectile(const vec &from, const vec &to, float speed, bool local, int id, fpsent *owner, int gun)
    {
        projectile &p = projs.add();
        p.dir = vec(to).sub(from).normalize();
        p.o = from;
        p.to = to;
        p.offset = hudgunorigin(gun, from, to, owner);
        p.offset.sub(from);
        p.speed = speed;
        p.local = local;
        p.owner = owner;
        p.gun = gun;
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
    }

    void removeprojectiles(fpsent *owner)
    {
        int len = projs.length();
        loopi(len) if(projs[i].owner==owner) { projs.remove(i--); len--; }
    }

    void damageeffect(int damage, fpsent *d, bool thirdperson)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        if(blood) particle_splash(PART_BLOOD, damage/10, 1000, p, ~bloodcolor, bloodintensity); // Fanatic Edition
        if(thirdperson)
        {
            defformatstring(ds)("%d", damage);
            particle_textcopy(d->abovehead(), ds, PART_TEXT, 2000, 0xFF4B19, 4.0f, -8);
        }
    }

    void spawnbouncer(const vec &p, const vec &vel, fpsent *d, int type, entitylight *light = NULL)
    {
        vec to(rnd(100)-50, rnd(100)-50, rnd(100)-50);
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, rnd(1000)+1000, rnd(100)+20, light);
    }

    void gibeffect(int damage, const vec &vel, fpsent *d)
    {
        if(!blood || damage <= 0) return;
        vec from = d->abovehead();
        loopi(min(damage/25, 40)+1) spawnbouncer(from, vel, d, BNC_GIBS);
    }

    void hit(int damage, dynent *d, fpsent *at, const vec &vel, int gun, float info1, int info2 = 1)
    {
        if(at==player1 && d!=at)
        {
            extern int hitsound;
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }

        if(d->type==ENT_INANIMATE)
        {
            hitmovable(damage, (movable *)d, at, vel, gun);
            return;
        }

        fpsent *f = (fpsent *)d;

        f->lastpain = lastmillis;
        if(at->type==ENT_PLAYER && !isteam(at->team, f->team)) at->totaldamage += damage;

        if(f->type==ENT_AI || !m_mp(gamemode) || f==at) f->hitpush(damage, vel, at, gun);

        if(f->type==ENT_AI) hitmonster(damage, (monster *)f, at, vel, gun);
        else if(!m_mp(gamemode)) damaged(damage, f, at);
        else
        {
            hitmsg &h = hits.add();
            h.target = f->clientnum;
            h.lifesequence = f->lifesequence;
            h.info1 = int(info1*DMF);
            h.info2 = info2;
            h.dir = f==at ? ivec(0, 0, 0) : ivec(vec(vel).mul(DNF));
            // Start: Fanatic Edition
            if(at==player1)
            {
                damageeffect(damage, f);
                if(f==player1)
                {
                    damageblend(damage);
                    damagecompass(damage, at ? at->o : f->o);
                    if(gun==GUN_FIST) playsound(S_SCH1+rnd(3));
                    else playsound(S_PAIN6);
                }
                else
                {
                    if(gun==GUN_FIST) playsound(S_SCH1+rnd(3), &f->o);
                    else playsound(S_PAIN1+rnd(5), &f->o);
                }
            }
            // End: Fanatic Edition
        }
    }

    void hitpush(int damage, dynent *d, fpsent *at, vec &from, vec &to, int gun, int rays)
    {
        hit(damage, d, at, vec(to).sub(from).normalize(), gun, from.dist(to), rays);
    }

    float projdist(dynent *o, vec &dir, const vec &v)
    {
        vec middle = o->o;
        middle.z += (o->aboveeye-o->eyeheight)/2;
        float dist = middle.dist(v, dir);
        dir.div(dist);
        if(dist<0) dist = 0;
        return dist;
    }

    void radialeffect(dynent *o, const vec &v, int qdam, fpsent *at, int gun)
    {
        if(o->state!=CS_ALIVE) return;
        vec dir;
        float dist = projdist(o, dir, v);
        if(dist<guns[gun].exprad)
        {
            int damage = (int)(qdam*(1-dist/EXP_DISTSCALE/guns[gun].exprad));
            if(o==at) damage /= EXP_SELFDAMDIV;
            hit(damage, o, at, dir, gun, dist);
        }
    }

    void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int damage, int gun)
    {
        // Start: Fanatic Edition
        particle_splash(PART_SPARK, 200, 300, v, 0xB49B4B, 0.24f);

        playsound(gun!=GUN_GL ? S_RLHIT : S_FEXPLODE, &v);

        int teamsmokecolor;
        if(!m_teammode) teamsmokecolor = teamcolorgreen;
        else
        {
            if(!joinred) teamsmokecolor = isteam(owner->team, player1->team) ? teamcolorblue : teamcolorred;
            else teamsmokecolor = isteam(owner->team, player1->team) ? teamcolorred : teamcolorblue;
        }
        particle_splash(PART_SMOKE, 5, 2500, v, smokecolorrainbow ? rnd(16777216) : (smokecolorteam ? teamsmokecolor : smokecolor), 12.0f, 50, 500);

        particle_fireball(v, guns[gun].exprad, gun!=GUN_GL ? PART_EXPLOSION : PART_EXPLOSION_BLUE, gun!=GUN_GL ? -1 : int((guns[gun].exprad-4.0f)*15), gun!=GUN_GL ? 0xFF8080 : 0x80FFFF, 4.0f);
        // End: Fanatic Edition
        if(gun == GUN_RL) adddynlight(v, 1.15f*guns[gun].exprad, vec(2, 1.5f, 1), 700, 100, 0, guns[gun].exprad/2, vec(1, 0.75f, 0.5f));
        else if(gun == GUN_GL) adddynlight(v, 1.15f*guns[gun].exprad, vec(0.5f, 1.5f, 2), 600, 100, 0, 8, vec(0.25f, 1, 1));
        else adddynlight(v, 1.15f*guns[gun].exprad, vec(2, 1.5f, 1), 700, 100);
        int numdebris = gun==GUN_BARREL ? rnd(max(maxbarreldebris-5, 1))+5 : rnd(maxdebris-5)+5;
        vec debrisvel = owner->o==v ? vec(0, 0, 0) : vec(owner->o).sub(v).normalize(), debrisorigin(v);
        if(gun==GUN_RL) debrisorigin.add(vec(debrisvel).mul(8));
        if(numdebris)
        {
            entitylight light;
            lightreaching(debrisorigin, light.color, light.dir);
            loopi(numdebris)
                spawnbouncer(debrisorigin, debrisvel, owner, gun==GUN_BARREL ? BNC_BARRELDEBRIS : BNC_DEBRIS, &light);
        }
        if(!local) return;
        int numdyn = numdynents();
        loopi(numdyn)
        {
            dynent *o = iterdynents(i);
            if(o->o.reject(v, o->radius + guns[gun].exprad) || o==safe) continue;
            radialeffect(o, v, damage, owner, gun);
        }
    }

    void projsplash(projectile &p, vec &v, dynent *safe, int damage)
    {
        if(guns[p.gun].part)
        {
            particle_splash(PART_SPARK, 100, 200, v, 0xB49B4B, 0.24f);
            playsound(S_FEXPLODE, &v);
        }
        else
        {
            explode(p.local, p.owner, v, safe, damage, GUN_RL);
            adddecal(DECAL_SCORCH, v, vec(p.dir).neg(), guns[p.gun].exprad/2);
        }
    }

    void explodeeffects(int gun, fpsent *d, bool local, int id)
    {
        if(local) return;
        switch(gun)
        {
            case GUN_RL:
                loopv(projs)
                {
                    projectile &p = projs[i];
                    if(p.gun == gun && p.owner == d && p.id == id && !p.local)
                    {
                        vec pos(p.o);
                        pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
                        explode(p.local, p.owner, pos, NULL, 0, GUN_RL);
                        adddecal(DECAL_SCORCH, pos, vec(p.dir).neg(), guns[gun].exprad/2);
                        projs.remove(i);
                        break;
                    }
                }
                break;
            case GUN_GL:
                loopv(bouncers)
                {
                    bouncer &b = *bouncers[i];
                    if(b.bouncetype == BNC_GRENADE && b.owner == d && b.id == id && !b.local)
                    {
                        vec pos(b.o);
                        pos.add(vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)));
                        explode(b.local, b.owner, pos, NULL, 0, GUN_GL);
                        adddecal(DECAL_SCORCH, pos, vec(0, 0, 1), guns[gun].exprad/2);
                        delete bouncers.remove(i);
                        break;
                    }
                }
                break;
            default:
                break;
        }
    }

    bool projdamage(dynent *o, projectile &p, vec &v, int qdam)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v)) return false;
        projsplash(p, v, o, qdam);
        vec dir;
        projdist(o, dir, v);
        hit(qdam, o, p.owner, dir, p.gun, 0);
        return true;
    }

    void updateprojectiles(int time)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            p.offsetmillis = max(p.offsetmillis-time, 0);
            int qdam = guns[p.gun].damage*(p.owner->quadmillis ? 4 : 1);
            if(p.owner->type==ENT_AI) qdam /= MONSTERDAMAGEFACTOR;
            vec dv;
            float dist = p.to.dist(p.o, dv); 
            dv.mul(time/max(dist*1000/p.speed, float(time)));
            vec v = vec(p.o).add(dv);
            bool exploded = false;
            hits.setsize(0);

            // Start: Fanatic Edition
            int teamsmokecolor;
            if(!m_teammode) teamsmokecolor = teamcolorgreen;
            else
            {
                if(!joinred) teamsmokecolor = isteam(p.owner->team, player1->team) ? teamcolorblue : teamcolorred;
                else teamsmokecolor = isteam(p.owner->team, player1->team) ? teamcolorred : teamcolorblue;
            }
            // End: Fanatic Edition

            if(p.local)
            {
                vec halfdv = vec(dv).mul(0.5f), bo = vec(p.o).add(halfdv);
                float br = max(fabs(halfdv.x), fabs(halfdv.y)) + 1;
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    if(p.owner==o || o->o.reject(bo, o->radius + br)) continue;
                    if(projdamage(o, p, v, qdam)) { exploded = true; break; }
                }
            }
            if(!exploded)
            {
                if(dist<4)
                {
                    if(p.o!=p.to)
                    {
                        if(raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT|RAY_ALPHAPOLY)>=4) continue;
                    }
                    projsplash(p, v, NULL, qdam);
                    exploded = true;
                }
                else
                {
                    vec pos(v);
                    pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
                    if(guns[p.gun].part)
                    {
                         int color = 0xFFFFFF;
                         switch(guns[p.gun].part)
                         {
                            case PART_FIREBALL1: color = 0xFFC8C8; break;
                         }
                         particle_splash(guns[p.gun].part, 1, 1, pos, color, 4.8f, 150, 20);
                    }
                    // Start: Fanatic Edition
                    else regular_particle_splash(PART_SMOKE, 2, 400, pos, smokecolorrainbow ? rnd(16777216) : (smokecolorteam ? teamsmokecolor : smokecolor), 2.4f, 50, -20);
                    if(lookupmaterial(pos) == MAT_WATER) regular_particle_splash(PART_BUBBLE, 4, 500, pos, 0xFFFFFF, 0.5f, 25, 500);
                    else particle_flare(pos, pos, 1, PART_MUZZLE_FLASH3, 0xFFFFFF, 1.0f + rndscale(5), NULL);
                    // FIXME: p.pc = playsound(S_ROCKET, &pos, NULL, 0, -1, 1, p.pc, -1, 3000);
                    // End: Fanatic Edition
                }
            }
            if(exploded)
            {
                if(p.local)
                    addmsg(N_EXPLODE, "rci3iv", p.owner, lastmillis-maptime, p.gun, p.id-maptime,
                            hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                projs.remove(i--);
                // FIXME: stopsound(S_ROCKET, p.pc, 3000); // Fanatic Edition
            }
            else p.o = v;
        }
    }

    extern int chainsawhudgun;

    // Start: Fanatic Edition
    VARP(playnearmisssound, 0, 1, 1);

    int sgnearmiss[3];

    void sound_nearmiss(int sound, const vec &s, const vec &e, bool limit = false)
    {
        if(!playnearmisssound) return;
        vec v;
        float d = e.dist(s, v);
        int steps = clamp(int(d*2), 1, 2048);
        v.div(steps);
        vec p = s;
        bool soundused = false;
        loopi(steps)
        {
            p.add(v);
            if(camera1->o.dist(p) <= 32)
            {
                if(limit)
                {
                    loopi(3)
                    {
                        if(!sgnearmiss[i])
                        {
                            sgnearmiss[i] = playsound(sound, &p, NULL, NULL, NULL, sgnearmiss[i]);
                            return;
                        }
                    }
                }
                else if(!soundused)
                {
                    playsound(sound, &p);
                    soundused = true;
                    return;
                }
            }
        }
    }

    // End: Fanatic Edition

    VARP(muzzleflash, 0, 1, 1);
    VARP(muzzlelight, 0, 1, 1);

    // Start: Fanatic Edition
    void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction)
    {
        int sound = guns[gun].sound, pspeed = 25;

        int teamsmokecolor;
        if(!m_teammode) teamsmokecolor = 0x32FF64;
        else
        {
            if(!joinred) teamsmokecolor = isteam(d->team, player1->team) ? 0x2222FF : 0xFF2222;
            else teamsmokecolor = isteam(d->team, player1->team) ? 0xFF2222 : 0x2222FF;
        }

        switch(gun)
        {
            case GUN_FIST:
                if(d->type==ENT_PLAYER && chainsawhudgun) sound = S_CHAINSAW_ATTACK;
                break;

            case GUN_SG:
            {
                if(!local) createrays(gun, from, to);
                if(muzzleflash && d->muzzle.x >= 0)
                {
                    particle_splash(PART_SMOKE, 3, 500, d->muzzle, smokecolorrainbow ? rnd(16777216) : (smokecolorteam ? teamsmokecolor : smokecolor), 1.4f, 50, 501);
                    particle_flare(d->muzzle, d->muzzle, 200, PART_MUZZLE_FLASH3, 0xFFFFFF, 2.75f, d);
                }
                loopi(guns[gun].rays)
                {
                    if(d!=hudplayer()) sound_nearmiss(S_NEARMISS1+rnd(3), from, rays[i], true);
                    if(shotsparks) particle_splash(PART_SPARK, 20, 250, rays[i], 0xB49B4B, 0.24f);
                    particle_flare(hudgunorigin(gun, from, rays[i], d), rays[i], sgshotduration, PART_STREAK, sgshotcolor, sgshotsize);
                    if(lookupmaterial(rays[i]) == MAT_WATER) regular_particle_splash(PART_BUBBLE, 1, 500, rays[i], 0xFFFFFF, 1.0f, 25, 500); // Fanatic Edition
                    if(!local) adddecal(DECAL_BULLET, rays[i], vec(from).sub(rays[i]).normalize(), 2.0f);
                }
                if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), 30, vec(0.5f, 0.375f, 0.25f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                break;
            }

            case GUN_CG:
                particle_flare(hudgunorigin(gun, from, to, d), to, cgshotduration, PART_STREAK, cgshotcolor, cgshotsize);
                particle_splash(PART_SMOKE, 3, 500, d->muzzle, smokecolorrainbow ? rnd(16777216) : (smokecolorteam ? teamsmokecolor : smokecolor), 1.4f, 50, 501);
            case GUN_PISTOL:
            {
                if(d!=hudplayer()) sound_nearmiss(S_NEARMISS1+rnd(3), from, to);
                if(shotsparks) particle_splash(PART_SPARK, 200, 250, to, 0xB49B4B, 0.24f);
                particle_flare(hudgunorigin(gun, from, to, d), to, pishotduration, PART_STREAK, pishotcolor, pishotsize);
                if(lookupmaterial(hudgunorigin(gun, from, to, d)) == MAT_WATER) regular_particle_splash(PART_BUBBLE, 1, 500, hudgunorigin(gun, from, to, d), 0xFFFFFF, 1.0f, 25, 500); // Fanatic Edition
                if(muzzleflash && d->muzzle.x >= 0)
                    particle_flare(d->muzzle, d->muzzle, gun==GUN_CG ? 100 : 200, PART_MUZZLE_FLASH1, 0xFFFFFF, gun==GUN_CG ? 2.25f : 1.25f, d);
                if(!local) adddecal(DECAL_BULLET, to, vec(from).sub(to).normalize(), 2.0f);
                if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), gun==GUN_CG ? 30 : 15, vec(0.5f, 0.375f, 0.25f), gun==GUN_CG ? 50 : 100, gun==GUN_CG ? 50 : 100, DL_FLASH, 0, vec(0, 0, 0), d);
                break;
            }

            case GUN_RL:
                if(muzzleflash && d->muzzle.x >= 0) particle_flare(d->muzzle, d->muzzle, 250, PART_MUZZLE_FLASH2, 0xFFFFFF, 3.0f, d);
                if(muzzlelight) adddynlight(d->o, 30, vec(1.0f, 1.0f, 0.5f), 500, 100, DL_FLASH, 0, vec(0, 0, 0), d);

            case GUN_FIREBALL:
            case GUN_ICEBALL:
            case GUN_SLIMEBALL:
                pspeed = guns[gun].projspeed;
                if(d->type==ENT_AI) pspeed /= 2;
                newprojectile(from, to, (float)pspeed, local, id, d, gun);
                break;

            case GUN_GL:
            {
                float dist = from.dist(to);
                vec up = to;
                up.z += dist/8;
                if(muzzleflash && d->muzzle.x >= 0) particle_flare(d->muzzle, d->muzzle, 200, PART_MUZZLE_FLASH2, 0xFFFFFF, 1.5f, d);
                if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), 20, vec(0.5f, 0.375f, 0.25f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
                newbouncer(from, up, local, id, d, BNC_GRENADE, guns[gun].ttl, guns[gun].projspeed);
                break;
            }

            case GUN_RIFLE:
                // Start: Fanatic Edition
                if(d != hudplayer()) sound_nearmiss(S_NEARMISS1+rnd(3), from, to);
                if(shotsparks) particle_splash(PART_SPARK, 200, 250, to, 0xB49B4B, 0.24f);
                if(ritraillaser)
                {
                    particle_flare(hudgunorigin(gun, from, to, d), to, 1500, PART_STREAK, lasercolorrainbow ? rnd(16777216) : (lasercolorteam ? teamsmokecolor : lasercolor), 0.6);
                }
                if(ritraillightning)
                {
                    particle_flare(hudgunorigin(gun, from, to, d), to, 500, PART_LIGHTNING, lightningcolorrainbow ? rnd(16777216) : (lightningcolorteam ? teamsmokecolor : lightningcolor), 0.6);
                }
                if(ritrailsmoke)
                {
                    particle_trail(PART_SMOKE, rishotduration, hudgunorigin(gun, from, to, d), to, smokecolorrainbow ? rnd(16777216) : (smokecolorteam ? teamsmokecolor : smokecolor), rishotsize, rishotgravity);
                    particle_flare(hudgunorigin(gun, from, to, d), to, riflareduration, PART_STREAK, riflarecolor, riflaresize);
                }
                if(ritrailspin)
                {
                    particle_trail_spin(PART_SPARK, 500, hudgunorigin(gun, from, to, d), to, spincolorrainbow ? rnd(16777216) : (spincolorteam ? teamsmokecolor : spincolor), 0.2, 0, 2.8, 0.1f);
                }
                if(lookupmaterial(hudgunorigin(gun, from, to, d)) == MAT_WATER) regular_particle_splash(PART_BUBBLE, 1, 500, hudgunorigin(gun, from, to, d), 0xFFFFFF, 1.0f, 25, 500);
                if(muzzleflash && d->muzzle.x >= 0) particle_flare(d->muzzle, d->muzzle, 150, PART_MUZZLE_FLASH3, 0xFFFFFF, 1.25f, d);
                if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), 25, vec(0.5f, 0.375f, 0.25f), 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
                if(!local) adddecal(DECAL_BULLET, to, vec(from).sub(to).normalize(), 3.0f);
                break;
                // End: Fanatic Edition
        }

        bool looped = false;
        if(d->attacksound >= 0 && d->attacksound != sound) d->stopattacksound();
        if(d->idlesound >= 0) d->stopidlesound();
        switch(sound)
        {
            case S_CHAINSAW_ATTACK:
                if(d->attacksound >= 0) looped = true;
                d->attacksound = sound;
                d->attackchan = playsound(sound, d==hudplayer() ? NULL : &d->o, NULL, 0, -1, 100, d->attackchan);
                break;
           case S_FLAUNCH:
                if(!ducky) playsound(S_FLAUNCH, d==hudplayer() ? NULL : &d->o);
                else playsound(S_DUCKY, d==hudplayer() ? NULL : &d->o);
                break;
            default:
                playsound(sound, d==hudplayer() ? NULL : &d->o);
                break;
        }
        if(d->quadmillis && lastmillis-prevaction>200 && !looped) playsound(S_ITEMPUP, d==hudplayer() ? NULL : &d->o);
    }
    // End: Fanatic Edition

    void particletrack(physent *owner, vec &o, vec &d)
    {
        if(owner->type!=ENT_PLAYER && owner->type!=ENT_AI) return;
        fpsent *pl = (fpsent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattackgun != pl->gunselect) return;
        float dist = o.dist(d);
        o = pl->muzzle;
        if(dist <= 0) d = o;
        else
        {
            vecfromyawpitch(owner->yaw, owner->pitch, 1, 0, d);
            float newdist = raycube(owner->o, d, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
            d.mul(min(newdist, dist)).add(owner->o);
        }
    }

    void dynlighttrack(physent *owner, vec &o, vec &hud)
    {
        if(owner->type!=ENT_PLAYER && owner->type!=ENT_AI) return;
        fpsent *pl = (fpsent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattackgun != pl->gunselect) return;
        o = pl->muzzle;
        hud = owner == hudplayer() ? vec(pl->o).add(vec(0, 0, 2)) : pl->muzzle;
    }

    float intersectdist = 1e16f;

    bool intersect(dynent *d, const vec &from, const vec &to, float &dist)
    {
        vec bottom(d->o), top(d->o);
        bottom.z -= d->eyeheight;
        top.z += d->aboveeye;
        return linecylinderintersect(from, to, bottom, top, d->radius, dist);
    }

    dynent *intersectclosest(const vec &from, const vec &to, fpsent *at, float &bestdist)
    {
        dynent *best = NULL;
        bestdist = 1e16f;
        loopi(numdynents())
        {
            dynent *o = iterdynents(i);
            if(o==at || o->state!=CS_ALIVE) continue;
            float dist;
            if(!intersect(o, from, to, dist)) continue;
            if(dist<bestdist)
            {
                best = o;
                bestdist = dist;
            }
        }
        return best;
    }

    void shorten(vec &from, vec &target, float dist)
    {
        target.sub(from).mul(min(1.0f, dist)).add(from);
    }

    void raydamage(vec &from, vec &to, fpsent *d)
    {
        int qdam = guns[d->gunselect].damage;
        if(d->quadmillis) qdam *= 4;
        if(d->type==ENT_AI) qdam /= MONSTERDAMAGEFACTOR;
        dynent *o;
        float dist;
        if(guns[d->gunselect].rays > 1)
        {
            dynent *hits[MAXRAYS];
            int maxrays = guns[d->gunselect].rays;
            loopi(maxrays) 
            {
                if((hits[i] = intersectclosest(from, rays[i], d, dist))) shorten(from, rays[i], dist);
                else adddecal(DECAL_BULLET, rays[i], vec(from).sub(rays[i]).normalize(), 2.0f);
            }
            loopi(maxrays) if(hits[i])
            {
                o = hits[i];
                hits[i] = NULL;
                int numhits = 1;
                for(int j = i+1; j < maxrays; j++) if(hits[j] == o)
                {
                    hits[j] = NULL;
                    numhits++;
                }
                hitpush(numhits*qdam, o, d, from, to, d->gunselect, numhits);
            }
        }
        else if((o = intersectclosest(from, to, d, dist)))
        {
            shorten(from, to, dist);
            hitpush(qdam, o, d, from, to, d->gunselect, 1);
        }
        else if(d->gunselect!=GUN_FIST && d->gunselect!=GUN_BITE) adddecal(DECAL_BULLET, to, vec(from).sub(to).normalize(), d->gunselect==GUN_RIFLE ? 3.0f : 2.0f);
    }

    void shoot(fpsent *d, const vec &targ)
    {
        int prevaction = d->lastaction, attacktime = lastmillis-prevaction;
        if(attacktime<d->gunwait) return;
        d->gunwait = 0;
        if((d==player1 || d->ai) && !d->attacking) return;
        d->lastaction = lastmillis;
        d->lastattackgun = d->gunselect;
        if(!d->ammo[d->gunselect])
        {
            if(d==player1)
            {
                msgsound(S_NOAMMO, d);
                d->gunwait = 600;
                d->lastattackgun = -1;
                weaponswitch(d);
            }
            return;
        }
        if(d->gunselect) d->ammo[d->gunselect]--;
        vec from = d->o;
        vec to = targ;

        vec unitv;
        float dist = to.dist(from, unitv);
        unitv.div(dist);
        vec kickback(unitv);
        kickback.mul(guns[d->gunselect].kickamount*-2.5f);
        d->vel.add(kickback);
        float shorten = 0;
        if(guns[d->gunselect].range && dist > guns[d->gunselect].range)
            shorten = guns[d->gunselect].range;
        float barrier = raycube(d->o, unitv, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
            shorten = barrier;
        if(shorten) to = vec(unitv).mul(shorten).add(from);

        if(guns[d->gunselect].rays > 1) createrays(d->gunselect, from, to);
        else if(guns[d->gunselect].spread) offsetray(from, to, guns[d->gunselect].spread, guns[d->gunselect].range, to);

        hits.setsize(0);

        if(!guns[d->gunselect].projspeed) raydamage(from, to, d);

        shoteffects(d->gunselect, from, to, d, true, 0, prevaction);

        if(d==player1 || d->ai)
        {
            addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, d->gunselect,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
        }

		d->gunwait = guns[d->gunselect].attackdelay;
		if(d->gunselect == GUN_PISTOL && d->ai) d->gunwait += int(d->gunwait*(((101-d->skill)+rnd(111-d->skill))/100.f));
        d->totalshots += guns[d->gunselect].damage*(d->quadmillis ? 4 : 1)*guns[d->gunselect].rays;
    }

    void adddynlights()
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            if(p.gun!=GUN_RL) continue;
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, 20, vec(1, 0.75f, 0.5f));
        }
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype!=BNC_GRENADE) continue;
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, 8, vec(0.25f, 1, 1));
        }
    }

    static const char * const projnames[3] = { "projectiles/grenade", "../../faned/projectiles/ducky", "projectiles/rocket" };
    static const char * const gibnames[3] = { "gibs/gib01", "gibs/gib02", "gibs/gib03" };
    static const char * const debrisnames[4] = { "debris/debris01", "debris/debris02", "debris/debris03", "debris/debris04" };
    static const char * const barreldebrisnames[4] = { "barreldebris/debris01", "barreldebris/debris02", "barreldebris/debris03", "barreldebris/debris04" };
         
    void preloadbouncers()
    {
        loopi(sizeof(projnames)/sizeof(projnames[0])) preloadmodel(projnames[i]);
        loopi(sizeof(gibnames)/sizeof(gibnames[0])) preloadmodel(gibnames[i]);
        loopi(sizeof(debrisnames)/sizeof(debrisnames[0])) preloadmodel(debrisnames[i]);
        loopi(sizeof(barreldebrisnames)/sizeof(barreldebrisnames[0])) preloadmodel(barreldebrisnames[i]);
    }

    void renderbouncers()
    {
        float yaw, pitch;
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            vec vel(bnc.vel);
            if(vel.magnitude() <= 25.0f) yaw = bnc.lastyaw;
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += 90;
                bnc.lastyaw = yaw;
            }
            pitch = -bnc.roll;
            if(bnc.bouncetype==BNC_GRENADE)
            {
                if(!ducky) rendermodel(&bnc.light, "projectiles/grenade", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHT|MDL_LIGHT_FAST|MDL_DYNSHADOW);
            else rendermodel(&bnc.light, "../../faned/projectiles/ducky", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHT|MDL_LIGHT_FAST|MDL_DYNSHADOW);
            }
            else
            {
                const char *mdl = NULL;
                int cull = MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;
                float fade = 1;
                if(bnc.lifetime < 250) fade = bnc.lifetime/250.0f;
                switch(bnc.bouncetype)
                {
                    case BNC_GIBS: mdl = gibnames[bnc.variant]; cull |= MDL_LIGHT|MDL_LIGHT_FAST|MDL_DYNSHADOW; break;
                    case BNC_DEBRIS: mdl = debrisnames[bnc.variant]; break;
                    case BNC_BARRELDEBRIS: mdl = barreldebrisnames[bnc.variant]; break;
                    default: continue;
                }
                rendermodel(&bnc.light, mdl, ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, cull, NULL, NULL, 0, 0, fade);
            }
        }
    }

    void renderprojectiles()
    {
        float yaw, pitch;
        loopv(projs)
        {
            projectile &p = projs[i];
            if(p.gun!=GUN_RL) continue;
            float dist = min(p.o.dist(p.to)/32.0f, 1.0f);
            vec pos = vec(p.o).add(vec(p.offset).mul(dist*p.offsetmillis/float(OFFSETMILLIS))),
                v = dist < 1e-6f ? p.dir : vec(p.to).sub(pos).normalize();
            // the amount of distance in front of the smoke trail needs to change if the model does
            vectoyawpitch(v, yaw, pitch);
            yaw += 90;
            v.mul(3);
            v.add(pos);
            rendermodel(&p.light, "projectiles/rocket", ANIM_MAPMODEL|ANIM_LOOP, v, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHT|MDL_LIGHT_FAST);
            particle_fireball(pos, 1.5f, PART_EXPLOSION, 15, 0xFF8080, 1.0f); // Fanatic Edition
        }
    }

    void checkattacksound(fpsent *d, bool local)
    {
        int gun = -1;
        switch(d->attacksound)
        {
            case S_CHAINSAW_ATTACK:
                if(chainsawhudgun) gun = GUN_FIST;
                break;
            default:
                return;
        }
        if(gun >= 0 && gun < NUMGUNS &&
           d->clientnum >= 0 && d->state == CS_ALIVE &&
           d->lastattackgun == gun && lastmillis - d->lastaction < guns[gun].attackdelay + 50)
        {
            d->attackchan = playsound(d->attacksound, local ? NULL : &d->o, NULL, 0, -1, -1, d->attackchan);
            if(d->attackchan < 0) d->attacksound = -1;
        }
        else d->stopattacksound();
    }

    void checkidlesound(fpsent *d, bool local)
    {
        int sound = -1, radius = 0;
        if(d->clientnum >= 0 && d->state == CS_ALIVE) switch(d->gunselect)
        {
            case GUN_FIST:
                if(chainsawhudgun && d->attacksound < 0)
                {
                    sound = S_CHAINSAW_IDLE;
                    radius = 50;
                }
                break;
        }
        if(d->idlesound != sound)
        {
            if(d->idlesound >= 0) d->stopidlesound();
            if(sound >= 0)
            {
                d->idlechan = playsound(sound, local ? NULL : &d->o, NULL, 0, -1, 100, d->idlechan, radius);
                if(d->idlechan >= 0) d->idlesound = sound;
            }
        }
        else if(sound >= 0)
        {
            d->idlechan = playsound(sound, local ? NULL : &d->o, NULL, 0, -1, -1, d->idlechan, radius);
            if(d->idlechan < 0) d->idlesound = -1;
        }
    }

    void removeweapons(fpsent *d)
    {
        removebouncers(d);
        removeprojectiles(d);
    }

    void updateweapons(int curtime)
    {
        updateprojectiles(curtime);
        if(player1->clientnum>=0 && player1->state==CS_ALIVE) shoot(player1, worldpos);
        updatebouncers(curtime);
        fpsent *following = followingplayer();
        if(!following) following = player1;
        loopv(players)
        {
            fpsent *d = players[i];
            checkattacksound(d, d==following);
            checkidlesound(d, d==following);
        }
    }

    void avoidweapons(ai::avoidset &obstacles, float radius)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            obstacles.avoidnear(NULL, p.o.z + guns[p.gun].exprad + 1, p.o, radius + guns[p.gun].exprad);
        }
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype != BNC_GRENADE) continue;
            obstacles.avoidnear(NULL, bnc.o.z + guns[GUN_GL].exprad + 1, bnc.o, radius + guns[GUN_GL].exprad);
        }
    }
};

