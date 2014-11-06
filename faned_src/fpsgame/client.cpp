#include "game.h"

// Start: Fanatic Edition
extern void lua_N_DAMAGE(int acn, int tcn, int damage, int armour, int health);
extern void lua_N_DIED(int vcn, int acn, int frags, int tfrags);
extern void lua_N_EDITMODE(int cn, int val);
extern void lua_N_EVENT(int event, int arg1, int arg2, int arg3, int arg4);
extern void lua_N_SAYTEAM(int cn, char * text);
extern void lua_N_SERVCMD(int cn, char * text);
extern void lua_N_SHOTFX(int cn, int gun, vec *to);
extern void lua_N_TEXT(int cn, char * text);

namespace game
{
    VARP(debugaim, 0, 0, 1);    // Fanatic Edition
    VARP(debugdamage, 0, 0, 1); // Fanatic Edition
    VARP(debugshoot, 0, 0, 1);  // Fanatic Edition

    FVARP(minimapalpha, 0, 0.5f, 1);
    VARP(maxradarscale, 1, 1024, 10000);
    VARP(minradarscale, 0, 384, 10000);
    VARP(radarteammates, 0, 1, 1);
    VARP(radarteammatesdead, 0, 0, 1); // Fanatic Edition
    VARP(radarteammatesplayerstarts, 0, 1, 1); // Fanatic Edition

    float calcradarscale()
    {
        return clamp(max(minimapradius.x, minimapradius.y)/3, float(minradarscale), float(maxradarscale));
    }

    void drawminimap(fpsent *d, float x, float y, float s)
    {
        vec pos = vec(d->o).sub(minimapcenter).mul(minimapscale).add(0.5f), dir;
        vecfromyawpitch(camera1->yaw, 0, 1, 0, dir);
        float scale = calcradarscale();
        glBegin(GL_TRIANGLE_FAN);
        loopi(16)
        {
            vec tc = vec(dir).rotate_around_z(i/16.0f*2*M_PI);
            glTexCoord2f(pos.x + tc.x*scale*minimapscale.x, pos.y + tc.y*scale*minimapscale.y);
            vec v = vec(0, -1, 0).rotate_around_z(i/16.0f*2*M_PI);
            glVertex2f(x + 0.5f*s*(1.0f + v.x), y + 0.5f*s*(1.0f + v.y));
        }
        glEnd();
    }

    void drawradar(float x, float y, float s)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x,   y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x+s, y);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x,   y+s);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x+s, y+s);
        glEnd();
    }

    // Start: Fanatic Edition
    void drawteammate(fpsent *d, float x, float y, float s, fpsent *o, float scale)
    {
        float height = 0.06f;
        float factor = 0.02f;
        float diff = 50;
        vec dir = d->o;
        dir.sub(o->o).div(scale);
        float dist = dir.magnitude2(), maxdist = 1 - 0.05f - 0.05f;
        if(dist >= maxdist) dir.mul(maxdist/dist);
        dir.rotate_around_z(-camera1->yaw*RAD);
        float bz = o->o.z - player1->o.z;
        if(bz > diff) bz = diff;
        if(bz < -diff) bz = -diff;
        bz = (height-(factor/2)) + ((1-pow(-bz/diff, 3))*factor);
        float bs = bz*s,
        bx = x + s*0.5f*(1.0f + dir.x),
        by = y + s*0.5f*(1.0f + dir.y);
        vec v(-0.5f, -0.5f, 0);
        v.rotate_around_z((90+o->yaw-camera1->yaw)*RAD);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(bx + bs*v.x, by + bs*v.y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(bx + bs*v.y, by - bs*v.x);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(bx - bs*v.x, by - bs*v.y);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(bx - bs*v.y, by + bs*v.x);
    }

    void drawplayerstartblip(fpsent *d, float x, float y, float s, const vec &pos, bool cutoff = false)
    {
        float scale = calcradarscale();
        vec dir = d->o;
        dir.sub(pos).div(scale);
        float size = 0.025f,
              xoffset = -size,
              yoffset = -size,
              dist = dir.magnitude2(), maxdist = 1 - 0.05f - 0.05f;
        if(dist >= maxdist) dir.mul(maxdist/dist);
        dir.rotate_around_z(-camera1->yaw*RAD);
        if(dist < maxdist || (dist >= maxdist && cutoff == false)) drawradar(x + s*0.5f*(1.0f + dir.x + xoffset), y + s*0.5f*(1.0f + dir.y + yoffset), size*s);
    }

    void drawteammates(fpsent *d, float x, float y, float s)
    {
        float scale = calcradarscale();
        int alive = 0, dead = 0;
        if(!joinred)
        {
            if(radarteammates)
            {
                loopv(players) 
                {
                    fpsent *o = players[i];
                    if(o != d && o->state == CS_ALIVE && isteam(o->team, d->team))
                    {
                        if(!alive++) 
                        {
                            settexture(isteam(d->team, player1->team) ? "packages/hud/blip_blue_alive.png" : "packages/hud/blip_red_alive.png");
                            glBegin(GL_QUADS);
                        }
                        drawteammate(d, x, y, s, o, scale);
                    }
                }
                if(alive) glEnd();
            }
            
            if(radarteammatesdead)
            {
                loopv(players) 
                {
                    fpsent *o = players[i];
                    if(o != d && o->state == CS_DEAD && isteam(o->team, d->team))
                    {
                        if(!dead++) 
                        {
                            settexture(isteam(d->team, player1->team) ? "packages/hud/blip_blue_dead.png" : "packages/hud/blip_red_dead.png");
                            glBegin(GL_QUADS);
                        }
                        drawteammate(d, x, y, s, o, scale);
                    }
                }
                if(dead) glEnd();
            }

            if(radarteammatesplayerstarts)
            {
                const vector<extentity *> &playerstarts = entities::getents();

                loopv(playerstarts)
                {
                    if(cmode && playerstarts[i]->type == ET_PLAYERSTART)
                    {
                        if(strcmp(d->team, "good") == 0 && playerstarts[i]->attr2 == 1)
                        {
                            settexture(isteam(d->team, player1->team) ? "faned/hud/blip_blue.png" : "faned/hud/blip_red.png");
                            drawplayerstartblip(d, x, y, s, playerstarts[i]->o, true);
                        }

                        else if(strcmp(d->team, "evil") == 0 && playerstarts[i]->attr2 == 2)
                        {
                            settexture(isteam(d->team, player1->team) ? "faned/hud/blip_blue.png" : "faned/hud/blip_red.png");
                            drawplayerstartblip(d, x, y, s, playerstarts[i]->o, true);
                        }
                    }
                }
            }
        }
        else
        {
            if(radarteammates)
            {
                loopv(players) 
                {
                    fpsent *o = players[i];
                    if(o != d && o->state == CS_ALIVE && isteam(o->team, d->team))
                    {
                        if(!alive++) 
                        {
                            settexture(isteam(d->team, player1->team) ? "packages/hud/blip_red_alive.png" : "packages/hud/blip_blue_alive.png");
                            glBegin(GL_QUADS);
                        }
                        drawteammate(d, x, y, s, o, scale);
                    }
                }
                if(alive) glEnd();
            }
            
            if(radarteammatesdead)
            {
                loopv(players) 
                {
                    fpsent *o = players[i];
                    if(o != d && o->state == CS_DEAD && isteam(o->team, d->team))
                    {
                        if(!dead++) 
                        {
                            settexture(isteam(d->team, player1->team) ? "packages/hud/blip_red_dead.png" : "packages/hud/blip_blue_dead.png");
                            glBegin(GL_QUADS);
                        }
                        drawteammate(d, x, y, s, o, scale);
                    }
                }
                if(dead) glEnd();
            }

            if(radarteammatesplayerstarts)
            {
                const vector<extentity *> &playerstarts = entities::getents();

                loopv(playerstarts)
                {
                    if(cmode && playerstarts[i]->type == ET_PLAYERSTART)
                    {
                        if(strcmp(d->team, "good") == 0 && playerstarts[i]->attr2 == 1)
                        {
                            settexture(isteam(d->team, player1->team) ? "faned/hud/blip_red.png" : "faned/hud/blip_blue.png");
                            drawplayerstartblip(d, x, y, s, playerstarts[i]->o, true);
                        }

                        else if(strcmp(d->team, "evil") == 0 && playerstarts[i]->attr2 == 2)
                        {
                            settexture(isteam(d->team, player1->team) ? "faned/hud/blip_red.png" : "faned/hud/blip_blue.png");
                            drawplayerstartblip(d, x, y, s, playerstarts[i]->o, true);
                        }
                    }
                }
            }
        }
    }
        
    #include "capture.h"
    #include "ctf.h"
    #include "collect.h"

    clientmode *cmode = NULL;
    captureclientmode capturemode;
    ctfclientmode ctfmode;
    collectclientmode collectmode;

    void setclientmode()
    {
        if(m_capture) cmode = &capturemode;
        else if(m_ctf) cmode = &ctfmode;
        else if(m_collect) cmode = &collectmode;
        else cmode = NULL;
    }

    bool senditemstoserver = false, sendcrc = false;
    int lastping = 0;

    bool connected = false, remote = false, demoplayback = false, gamepaused = false;
    int sessionid = 0, mastermode = MM_OPEN, gamespeed = 100;
    string servinfo = "", servauth = "", connectpass = "";

    VARP(deadpush, 1, 2, 20);

    void switchname(const char *name)
    {
        filtertext(player1->name, name, false, MAXNAMELEN);
        if(!player1->name[0]) copystring(player1->name, "unnamed");
        conoutf("you are now known as %s", player1->name); // Fanatic Edition
        addmsg(N_SWITCHNAME, "rs", player1->name);
    }
    void printname()
    {
        conoutf("your name is: %s", colorname(player1));
    }
    ICOMMAND(name, "sN", (char *s, int *numargs),
    {
        if(*numargs > 0) switchname(s);
        else if(!*numargs) printname();
        else result(player1->name); // Fanatic Edition
    });
    ICOMMAND(getname, "", (), result(player1->name));

    void switchteam(const char *team)
    {
        if(player1->clientnum < 0) filtertext(player1->team, team, false, MAXTEAMLEN);
        else addmsg(N_SWITCHTEAM, "rs", team);
    }
    void printteam()
    {
        conoutf("your team is: %s", player1->team);
    }
    ICOMMAND(team, "sN", (char *s, int *numargs),
    {
        if(*numargs > 0) switchteam(s);
        else if(!*numargs) printteam();
        else result(player1->team);
    });
    ICOMMAND(getteam, "", (), result(player1->team));

    void switchplayermodel(int playermodel)
    {
        player1->playermodel = playermodel;
        addmsg(N_SWITCHMODEL, "ri", player1->playermodel);
    }

    struct authkey
    {
        char *name, *key, *desc;
        int lastauth;

        authkey(const char *name, const char *key, const char *desc)
            : name(newstring(name)), key(newstring(key)), desc(newstring(desc)),
              lastauth(0)
        {
        }

        ~authkey()
        {
            DELETEA(name);
            DELETEA(key);
            DELETEA(desc);
        }
    };
    vector<authkey *> authkeys;

    authkey *findauthkey(const char *desc = "")
    {
        loopv(authkeys) if(!strcmp(authkeys[i]->desc, desc) && !strcasecmp(authkeys[i]->name, player1->name)) return authkeys[i];
        loopv(authkeys) if(!strcmp(authkeys[i]->desc, desc)) return authkeys[i];
        return NULL;
    }

    VARP(autoauth, 0, 1, 1);

    void addauthkey(const char *name, const char *key, const char *desc)
    {
        loopvrev(authkeys) if(!strcmp(authkeys[i]->desc, desc) && !strcmp(authkeys[i]->name, name)) delete authkeys.remove(i);
        if(name[0] && key[0]) authkeys.add(new authkey(name, key, desc));
    }
    ICOMMAND(authkey, "sss", (char *name, char *key, char *desc), addauthkey(name, key, desc));

    bool hasauthkey(const char *name, const char *desc)
    {
        if(!name[0] && !desc[0]) return authkeys.length() > 0;
        loopvrev(authkeys) if(!strcmp(authkeys[i]->desc, desc) && !strcmp(authkeys[i]->name, name)) return true;
        return false;
    }

    ICOMMAND(hasauthkey, "ss", (char *name, char *desc), intret(hasauthkey(name, desc) ? 1 : 0));

    void genauthkey(const char *secret)
    {
        if(!secret[0])
        {
            conoutf(CON_ERROR, "you must specify a secret password");
            playsound(S_ERROR);
            return;
        }
        vector<char> privkey, pubkey;
        genprivkey(secret, privkey, pubkey);
        conoutf("private key: %s", privkey.getbuf());
        conoutf("public key: %s", pubkey.getbuf());
    }
    COMMAND(genauthkey, "s");

    // Start: Fanatic Edition
    void saveauthkeys()
    {
        stream *f = openfile("faned_auth.cfg", "w");
        if(!f)
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::saveauthkeys: \f3#error: \f7failed to open \f4faned_auth.cfg\f7 for writing");
            playsound(S_ERROR);
            return;
        }
        loopv(authkeys)
        {
            authkey *a = authkeys[i];
            f->printf("authkey %s %s %s\n", escapestring(a->name), escapestring(a->key), escapestring(a->desc));
        }
        conoutf(CON_INFO, "\f9FanEd\f7::saveauthkeys: saved authkeys to \f4faned_auth.cfg");
        delete f;
    }
    COMMAND(saveauthkeys, "");
    // End: Fanatic Edition

    void sendmapinfo()
    {
        if(!connected) return;
        sendcrc = true;
        if(player1->state!=CS_SPECTATOR || player1->privilege || !remote) senditemstoserver = true;
    }

    void writeclientinfo(stream *f)
    {
        f->printf("name %s\n", escapestring(player1->name));
    }

    bool allowedittoggle()
    {
        if(editmode) return true;
        if(isconnected() && multiplayer(false) && !m_edit)
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::edittoggle: \f3#error: \f7editing in multiplayer requires coop edit mode (1)");
            playsound(S_ERROR);
            return false;
        }
        if(identexists("allowedittoggle") && !execute("allowedittoggle")) return false;
        return true;
    }

    void edittoggled(bool on)
    {
        addmsg(N_EDITMODE, "ri", on ? 1 : 0);
        if(player1->state==CS_DEAD) deathstate(player1, true);
        else if(player1->state==CS_EDITING && player1->editstate==CS_DEAD) showscores(false);
        disablezoom();
        player1->suicided = player1->respawned = -2;
    }

    const char *getclientname(int cn)
    {
        fpsent *d = getclient(cn);
        return d ? d->name : "";
    }
    ICOMMAND(getclientname, "i", (int *cn), result(getclientname(*cn)));

    const char *getclientteam(int cn)
    {
        fpsent *d = getclient(cn);
        return d ? d->team : "";
    }
    ICOMMAND(getclientteam, "i", (int *cn), result(getclientteam(*cn)));

    int getclientmodel(int cn)
    {
        fpsent *d = getclient(cn);
        return d ? d->playermodel : -1;
    }
    ICOMMAND(getclientmodel, "i", (int *cn), intret(getclientmodel(*cn)));

    const char *getclienticon(int cn)
    {
        fpsent *d = getclient(cn);
        if(!d || d->state==CS_SPECTATOR) return "spectator";
        const playermodelinfo &mdl = getplayermodelinfo(d);
        return m_teammode ? (isteam(player1->team, d->team) ? mdl.blueicon : mdl.redicon) : mdl.ffaicon;
    }
    ICOMMAND(getclienticon, "i", (int *cn), result(getclienticon(*cn)));

    // Start: Fanatic Edition
    int getclientinfo(int cn, int type)
    {
        fpsent *d = getclient(cn);

        if(!d) return -1;

        switch(type)
        {   
            case  0: return 0;
            case  1: return int(d->ai);
            case  2: return int(d->armour); //  A_BLUE, A_GREEN, A_YELLOW
            case  3: return int(d->armourtype);
            case  4: return int(d->attackchan);
            case  5: return int(d->attacksound);
            case  6: return int(d->deaths);
            case  7: return int(d->edit); 
            case  8: return int(d->flagpickup);
            case  9: return int(d->flags);
            case 10: return int(d->frags);
            case 11: return int(d->gunselect); // GUN_FIST, GUN_SG, GUN_CG, GUN_RL, GUN_RIFLE, GUN_GL, GUN_PISTOL
            case 12: return int(d->health);
            case 13: return int(d->idlechan);
            case 14: return int(d->idlesound);
            case 15: return int(d->lastaction);
            case 16: return int(d->lastattackgun);
            case 17: return int(d->lastbase);
            case 18: return int(d->lastnode);
            case 19: return int(d->lastpain);
            case 20: return int(d->lastpickup);
            case 21: return int(d->lastpickupmillis);
            case 22: return int(d->lastrepammo);
            case 23: return int(d->lasttaunt);
            case 24: return int(d->lastupdate);
            case 25: return int(d->lifesequence);
            case 26: return int(d->ownernum);
            case 27: return int(d->o.x);
            case 28: return int(d->o.y);
            case 29: return int(d->o.z);
            case 30: return int(d->physstate); // PHYS_FLOAT, PHYS_FALL, PHYS_SLIDE, PHYS_SLOPE, PHYS_FLOOR, PHYS_STEP_UP, PHYS_STEP_DOWN, PHYS_BOUNCE
            case 31: return int(d->ping);
            case 32: return int(d->pitch);
            case 33: return int(d->plag);
            case 34: return int(d->playermodel);
            case 35: return int(d->privilege);
            case 36: return int(d->respawned);
            case 37: return int(d->smoothmillis);
            case 38: return int(d->state); // CS_ALIVE, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATOR
            case 39: return int(d->suicided);
            case 40: return int(d->suicides);
            case 41: return int(d->teamkills);
            case 42: return int(d->tokens);
            case 43: return int(d->totaldamage);
            case 44: return int(d->totalshots);
            case 45: return int(d->vel.magnitude());
            case 46: return int(d->vel.x);
            case 47: return int(d->vel.y);
            case 48: return int(d->vel.z);
            case 49: return int(d->weight);
            case 50: return int(d->yaw);
        }
        return 0;
    }

    ICOMMAND(getclientinfo, "ii", (int *cn, int *type), intret(getclientinfo(*cn, *type)));

    bool ismaster(int cn)
    {
        fpsent *d = getclient(cn);
        return d && d->privilege >= PRIV_MASTER;
    }
    ICOMMAND(ismaster, "i", (int *cn), intret(ismaster(*cn) ? 1 : 0));

    bool isauth(int cn)
    {
        fpsent *d = getclient(cn);
        return d && d->privilege >= PRIV_AUTH;
    }
    ICOMMAND(isauth, "i", (int *cn), intret(isauth(*cn) ? 1 : 0));

    bool isadmin(int cn)
    {
        fpsent *d = getclient(cn);
        return d && d->privilege >= PRIV_ADMIN;
    }
    ICOMMAND(isadmin, "i", (int *cn), intret(isadmin(*cn) ? 1 : 0));

    ICOMMAND(getmastermode, "", (), intret(mastermode));
    ICOMMAND(mastermodename, "i", (int *mm), result(server::mastermodename(*mm, "")));

    bool isspectator(int cn)
    {
        fpsent *d = getclient(cn);
        return d && d->state == CS_SPECTATOR;
    }
    ICOMMAND(isspectator, "i", (int *cn), intret(isspectator(*cn) ? 1 : 0));

    bool isai(int cn, int type)
    {
        fpsent *d = getclient(cn);
        int aitype = type > 0 && type < AI_MAX ? type : AI_BOT;
        return d && d->aitype == aitype;
    }
    ICOMMAND(isai, "ii", (int *cn, int *type), intret(isai(*cn, *type) ? 1 : 0));

    int parseplayer(const char *arg)
    {
        char *end;
        int n = strtol(arg, &end, 10);
        if(*arg && !*end)
        {
            if(n!=player1->clientnum && !clients.inrange(n)) return -1;
            return n;
        }
        loopv(players)
        {
            fpsent *o = players[i];
            if(!strcmp(arg, o->name)) return o->clientnum;
        }
        loopv(players)
        {
            fpsent *o = players[i];
            if(!strcasecmp(arg, o->name)) return o->clientnum;
        }
        return -1;
    }
    ICOMMAND(getclientnum, "s", (char *name), intret(name[0] ? parseplayer(name) : player1->clientnum));

    void listclients(bool local, bool bots)
    {
        vector<char> buf;
        string cn;
        int numclients = 0;
        if(local && connected)
        {
            formatstring(cn)("%d", player1->clientnum);
            buf.put(cn, strlen(cn));
            numclients++;
        }
        loopv(clients) if(clients[i] && (bots || clients[i]->aitype == AI_NONE))
        {
            formatstring(cn)("%d", clients[i]->clientnum);
            if(numclients++) buf.add(' ');
            buf.put(cn, strlen(cn));
        }
        buf.add('\0');
        result(buf.getbuf());
    }
    ICOMMAND(listclients, "bb", (int *local, int *bots), listclients(*local>0, *bots!=0));

    void clearbans()
    {
        addmsg(N_CLEARBANS, "r");
    }
    COMMAND(clearbans, "");

    void kick(const char *victim, const char *reason)
    {
        int vn = parseplayer(victim);
        if(vn>=0 && vn!=player1->clientnum) addmsg(N_KICK, "ris", vn, reason);
    }
    COMMAND(kick, "ss");

    void authkick(const char *desc, const char *victim, const char *reason)
    {
        authkey *a = findauthkey(desc);
        int vn = parseplayer(victim);
        if(a && vn>=0 && vn!=player1->clientnum) 
        {
            a->lastauth = lastmillis;
            addmsg(N_AUTHKICK, "rssis", a->desc, a->name, vn, reason);
        }
    }
    ICOMMAND(authkick, "ss", (const char *victim, const char *reason), authkick("", victim, reason));
    ICOMMAND(sauthkick, "ss", (const char *victim, const char *reason), if(servauth[0]) authkick(servauth, victim, reason));
    ICOMMAND(dauthkick, "sss", (const char *desc, const char *victim, const char *reason), if(desc[0]) authkick(desc, victim, reason));

    vector<int> editmutes;

    void editmute(int cn)
    {
        fpsent *d = getclient(cn);
        if(!d || d == player1) return;
        conoutf(CON_INFO, "\f9FanEd\f7::editmute: ignoring %s \f4(%d)", d->name, d->clientnum);
        if(editmutes.find(cn) < 0) editmutes.add(cn);
    }

    void uneditmute(int cn)
    {
        if(editmutes.find(cn) < 0) return;
        fpsent *d = getclient(cn);
        if(d) conoutf(CON_INFO, "\f9FanEd\f7::editmute: stopped ignoring %s \f4(%d)", d->name, d->clientnum);
        editmutes.removeobj(cn);
    }

    bool iseditmuted(int cn) { return editmutes.find(cn) >= 0; }

    ICOMMAND(editmute, "s", (char *arg), editmute(parseplayer(arg)));
    ICOMMAND(uneditmute, "s", (char *arg), uneditmute(parseplayer(arg))); 
    ICOMMAND(iseditmuted, "s", (char *arg), intret(iseditmuted(parseplayer(arg)) ? 1 : 0));

    vector<int> ignores;

    void ignore(int cn)
    {
        fpsent *d = getclient(cn);
        if(!d || d == player1) return;
        conoutf("ignoring %s", d->name);
        if(ignores.find(cn) < 0) ignores.add(cn);
    }

    void unignore(int cn)
    {
        if(ignores.find(cn) < 0) return;
        fpsent *d = getclient(cn);
        if(d) conoutf("stopped ignoring %s", d->name);
        ignores.removeobj(cn);
    }

    bool isignored(int cn) { return ignores.find(cn) >= 0; }

    ICOMMAND(ignore, "s", (char *arg), ignore(parseplayer(arg)));
    ICOMMAND(unignore, "s", (char *arg), unignore(parseplayer(arg))); 
    ICOMMAND(isignored, "s", (char *arg), intret(isignored(parseplayer(arg)) ? 1 : 0));

    void setteam(const char *arg1, const char *arg2)
    {
        int i = parseplayer(arg1);
        if(i>=0) addmsg(N_SETTEAM, "ris", i, arg2);
    }
    COMMAND(setteam, "ss");

    void hashpwd(const char *pwd)
    {
        if(player1->clientnum<0) return;
        string hash;
        server::hashpassword(player1->clientnum, sessionid, pwd, hash);
        result(hash);
    }
    COMMAND(hashpwd, "s");

    void setmaster(const char *arg, const char *who)
    {
        if(!arg[0]) return;
        int val = 1, cn = player1->clientnum;
        if(who[0])
        {
            cn = parseplayer(who);
            if(cn < 0) return;
        }
        string hash = "";
        if(!arg[1] && isdigit(arg[0])) val = parseint(arg);
        else 
        {
            if(cn != player1->clientnum) return;
            server::hashpassword(player1->clientnum, sessionid, arg, hash);
        }
        addmsg(N_SETMASTER, "riis", cn, val, hash);
    }
    COMMAND(setmaster, "ss");
    ICOMMAND(mastermode, "i", (int *val), addmsg(N_MASTERMODE, "ri", *val));

    bool tryauth(const char *desc)
    {
        authkey *a = findauthkey(desc);
        if(!a) return false;
        a->lastauth = lastmillis;
        addmsg(N_AUTHTRY, "rss", a->desc, a->name);
        return true;
    }
    ICOMMAND(auth, "s", (char *desc), tryauth(desc));
    ICOMMAND(sauth, "", (), if(servauth[0]) tryauth(servauth));
    ICOMMAND(dauth, "s", (char *desc), if(desc[0]) tryauth(desc));

    void togglespectator(int val, const char *who)
    {
        int i = who[0] ? parseplayer(who) : player1->clientnum;
        if(i>=0) addmsg(N_SPECTATOR, "rii", i, val);
    }
    ICOMMAND(spectator, "is", (int *val, char *who), togglespectator(*val, who));

    ICOMMAND(checkmaps, "", (), addmsg(N_CHECKMAPS, "r"));

    int gamemode = INT_MAX, nextmode = INT_MAX;
    string clientmap = "";

    void changemapserv(const char *name, int mode)
    {
        if(multiplayer(false) && !m_mp(mode))
        {
            conoutf(CON_ERROR, "mode %s (%d) not supported in multiplayer", server::modename(gamemode), gamemode);
            playsound(S_ERROR); // Fanatic Edition
            loopi(NUMGAMEMODES) if(m_mp(STARTGAMEMODE + i)) { mode = STARTGAMEMODE + i; break; }
        }

        gamemode = mode;
        nextmode = mode;
        if(editmode) toggleedit();
        if(m_demo) { entities::resetspawns(); return; }
        if((m_edit && !name[0]) || !load_world(name))
        {
            emptymap(0, true, name);
            senditemstoserver = false;
        }
        startgame();
    }

    void setmode(int mode)
    {
        if(multiplayer(false) && !m_mp(mode))
        {
            conoutf(CON_ERROR, "mode %s (%d) not supported in multiplayer",  server::modename(mode), mode);
            playsound(S_ERROR); // Fanatic Edition
            intret(0);
            return;
        }
        nextmode = mode;
        intret(1);
    }
    ICOMMAND(mode, "i", (int *val), setmode(*val));
    ICOMMAND(getmode, "", (), intret(gamemode));
    ICOMMAND(timeremaining, "i", (int *formatted), 
    {
        int val = max(maplimit - lastmillis, 0)/1000;
        if(*formatted)
        {
            defformatstring(str)("%d:%02d", val/60, val%60);
            result(str);
        }
        else intret(val);
    });
    ICOMMANDS("m_noitems", "i", (int *mode), { int gamemode = *mode; intret(m_noitems); });
    ICOMMANDS("m_noammo", "i", (int *mode), { int gamemode = *mode; intret(m_noammo); });
    ICOMMANDS("m_insta", "i", (int *mode), { int gamemode = *mode; intret(m_insta); });
    ICOMMANDS("m_tactics", "i", (int *mode), { int gamemode = *mode; intret(m_tactics); });
    ICOMMANDS("m_efficiency", "i", (int *mode), { int gamemode = *mode; intret(m_efficiency); });
    ICOMMANDS("m_capture", "i", (int *mode), { int gamemode = *mode; intret(m_capture); });
    ICOMMANDS("m_regencapture", "i", (int *mode), { int gamemode = *mode; intret(m_regencapture); });
    ICOMMANDS("m_ctf", "i", (int *mode), { int gamemode = *mode; intret(m_ctf); });
    ICOMMANDS("m_protect", "i", (int *mode), { int gamemode = *mode; intret(m_protect); });
    ICOMMANDS("m_hold", "i", (int *mode), { int gamemode = *mode; intret(m_hold); });
    ICOMMANDS("m_collect", "i", (int *mode), { int gamemode = *mode; intret(m_collect); });
    ICOMMANDS("m_teammode", "i", (int *mode), { int gamemode = *mode; intret(m_teammode); });
    ICOMMANDS("m_demo", "i", (int *mode), { int gamemode = *mode; intret(m_demo); });
    ICOMMANDS("m_edit", "i", (int *mode), { int gamemode = *mode; intret(m_edit); });
    ICOMMANDS("m_lobby", "i", (int *mode), { int gamemode = *mode; intret(m_lobby); });
    ICOMMANDS("m_sp", "i", (int *mode), { int gamemode = *mode; intret(m_sp); });
    ICOMMANDS("m_dmsp", "i", (int *mode), { int gamemode = *mode; intret(m_dmsp); });
    ICOMMANDS("m_classicsp", "i", (int *mode), { int gamemode = *mode; intret(m_classicsp); });

    void changemap(const char *name, int mode)
    {
        if(!remote)
        {
            server::forcemap(name, mode);
            if(!isconnected()) localconnect();
        }
        else addmsg(N_MAPVOTE, "rsi", name, mode);
    }
    void changemap(const char *name)
    {
        changemap(name, m_valid(nextmode) ? nextmode : (remote ? 0 : 1));
    }
    ICOMMAND(map, "s", (char *name), changemap(name));

    void forceedit(const char *name)
    {
        changemap(name, 1);
    }

    void newmap(int size)
    {
        addmsg(N_NEWMAP, "ri", size);
    }

    int needclipboard = -1;

    void sendclipboard()
    {
        uchar *outbuf = NULL;
        int inlen = 0, outlen = 0;
        if(!packeditinfo(localedit, inlen, outbuf, outlen))
        {
            outbuf = NULL;
            inlen = outlen = 0;
        }
        packetbuf p(16 + outlen, ENET_PACKET_FLAG_RELIABLE);
        putint(p, N_CLIPBOARD);
        putint(p, inlen);
        putint(p, outlen);
        if(outlen > 0) p.put(outbuf, outlen);
        sendclientpacket(p.finalize(), 1);
        needclipboard = -1;
    }

    void edittrigger(const selinfo &sel, int op, int arg1, int arg2, int arg3)
    {
        if(m_edit) switch(op)
        {
            case EDIT_FLIP:
            case EDIT_COPY:
            case EDIT_PASTE:
            case EDIT_DELCUBE:
            {
                switch(op)
                {
                    case EDIT_COPY: needclipboard = 0; break;
                    case EDIT_PASTE:
                        if(needclipboard > 0)
                        {
                            c2sinfo(true);
                            sendclipboard();
                        }
                        break;
                }
                addmsg(N_EDITF + op, "ri9i4",
                   sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                   sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner);
                break;
            }
            case EDIT_ROTATE:
            {
                addmsg(N_EDITF + op, "ri9i5",
                   sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                   sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                   arg1);
                break;
            }
            case EDIT_MAT:
            case EDIT_FACE:
            case EDIT_TEX:
            {
                addmsg(N_EDITF + op, "ri9i6",
                   sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                   sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                   arg1, arg2);
                break;
            }
            case EDIT_REPLACE:
            {
                addmsg(N_EDITF + op, "ri9i7",
                   sel.o.x, sel.o.y, sel.o.z, sel.s.x, sel.s.y, sel.s.z, sel.grid, sel.orient,
                   sel.cx, sel.cxs, sel.cy, sel.cys, sel.corner,
                   arg1, arg2, arg3);
                break;
            }
            case EDIT_REMIP:
            {
                addmsg(N_EDITF + op, "r");
                break;
            }
        }
    }

    VARP(printsvarescaped, 0, 0, 1); // Fanatic Edition

    void printvar(fpsent *d, ident *id)
    {
        if(id) switch(id->type)
        {
            case ID_VAR:
            {
                int val = *id->storage.i;
                string str;
                if(val < 0)
                    formatstring(str)("%d", val); 
                else if(id->flags&IDF_HEX && id->maxval==0xFFFFFF)
                    formatstring(str)("0x%.6X (%d, %d, %d)", val, (val>>16)&0xFF, (val>>8)&0xFF, val&0xFF);
                else
                    formatstring(str)(id->flags&IDF_HEX ? "0x%X" : "%d", val);
                conoutf("%s set map var \"%s\" to %s", colorname(d), id->name, str);
                break;
            }
            case ID_FVAR:
                conoutf("%s set map var \"%s\" to %s", colorname(d), id->name, floatstr(*id->storage.f));
                break;
            case ID_SVAR:
                if(printsvarescaped)
                {
                    conoutf("\f9FanEd\f7::printvar: %s set map var \"%s\" to \"%s\"", colorname(d), id->name, escapestring(*id->storage.s));
                    conoutf("\f9FanEd\f7::printvar: new map var \"%s\" is \"%s\"", id->name, *id->storage.s);
                }
                else conoutf("%s set map var \"%s\" to \"%s\"", colorname(d), id->name, *id->storage.s);
                break;
        }
    }

    void vartrigger(ident *id)
    {
        if(!m_edit) return;
        switch(id->type)
        {
            case ID_VAR:
                addmsg(N_EDITVAR, "risi", ID_VAR, id->name, *id->storage.i);
                break;

            case ID_FVAR:
                addmsg(N_EDITVAR, "risf", ID_FVAR, id->name, *id->storage.f);
                break;

            case ID_SVAR:
                addmsg(N_EDITVAR, "riss", ID_SVAR, id->name, *id->storage.s);
                break;
            default: return;
        }
        printvar(player1, id);
    }

    void pausegame(bool val)
    {
        if(!connected) return;
        if(!remote) server::forcepaused(val);
        else addmsg(N_PAUSEGAME, "ri", val ? 1 : 0);
    }
    ICOMMAND(pausegame, "i", (int *val), pausegame(*val > 0));
    ICOMMAND(paused, "iN$", (int *val, int *numargs, ident *id),
    { 
        if(*numargs > 0) pausegame(clampvar(id, *val, 0, 1) > 0); 
        else if(*numargs < 0) intret(gamepaused ? 1 : 0);
        else printvar(id, gamepaused ? 1 : 0); 
    });

    bool ispaused() { return gamepaused; }

    bool allowmouselook() { return !gamepaused || !remote || m_edit; }

    void changegamespeed(int val)
    {
        if(!connected) return;
        if(!remote) server::forcegamespeed(val);
        else addmsg(N_GAMESPEED, "ri", val);
    }
    ICOMMAND(gamespeed, "iN$", (int *val, int *numargs, ident *id),
    {
        if(*numargs > 0) changegamespeed(clampvar(id, *val, 10, 1000));
        else if(*numargs < 0) intret(gamespeed);
        else printvar(id, gamespeed);
    });

    int scaletime(int t) { return t*gamespeed; }

    vector<uchar> messages;
    int messagecn = -1, messagereliable = false;

    void addmsg(int type, const char *fmt, ...)
    {
        if(!connected) return;
        static uchar buf[MAXTRANS];
        ucharbuf p(buf, sizeof(buf));
        putint(p, type);
        int numi = 1, numf = 0, nums = 0, mcn = -1;
        bool reliable = false;
        if(fmt)
        {
            va_list args;
            va_start(args, fmt);
            while(*fmt) switch(*fmt++)
            {
                case 'r': reliable = true; break;
                case 'c':
                {
                    fpsent *d = va_arg(args, fpsent *);
                    mcn = !d || d == player1 ? -1 : d->clientnum;
                    break;
                }
                case 'v':
                {
                    int n = va_arg(args, int);
                    int *v = va_arg(args, int *);
                    loopi(n) putint(p, v[i]);
                    numi += n;
                    break;
                }

                case 'i':
                {
                    int n = isdigit(*fmt) ? *fmt++-'0' : 1;
                    loopi(n) putint(p, va_arg(args, int));
                    numi += n;
                    break;
                }
                case 'f':
                {
                    int n = isdigit(*fmt) ? *fmt++-'0' : 1;
                    loopi(n) putfloat(p, (float)va_arg(args, double));
                    numf += n;
                    break;
                }
                case 's': sendstring(va_arg(args, const char *), p); nums++; break;
            }
            va_end(args);
        }
        int num = nums || numf ? 0 : numi, msgsize = server::msgsizelookup(type);
        if(msgsize && num!=msgsize) { fatal("inconsistent msg size for %d (%d != %d)", type, num, msgsize); }
        if(reliable) messagereliable = true;
        if(mcn != messagecn)
        {
            static uchar mbuf[16];
            ucharbuf m(mbuf, sizeof(mbuf));
            putint(m, N_FROMAI);
            putint(m, mcn);
            messages.put(mbuf, m.length());
            messagecn = mcn;
        }
        messages.put(buf, p.length());
    }

    void connectattempt(const char *name, const char *password, const ENetAddress &address)
    {
        copystring(connectpass, password);
    }

    void connectfail()
    {
        memset(connectpass, 0, sizeof(connectpass));
    }

    void gameconnect(bool _remote)
    {
        remote = _remote;
        if(editmode) toggleedit();
    }

    void gamedisconnect(bool cleanup)
    {
        if(remote) stopfollowing();
        ignores.setsize(0);
        connected = remote = false;
        player1->clientnum = -1;
        sessionid = 0;
        mastermode = MM_OPEN;
        messages.setsize(0);
        messagereliable = false;
        messagecn = -1;
        player1->respawn();
        player1->lifesequence = 0;
        player1->state = CS_ALIVE;
        player1->privilege = PRIV_NONE;
        sendcrc = senditemstoserver = false;
        demoplayback = false;
        gamepaused = false;
        gamespeed = 100;
        clearclients(false);
        if(cleanup)
        {
            nextmode = gamemode = INT_MAX;
            clientmap[0] = '\0';
        }
    }

    // Start: Fanatic Edition
    VARP(playchatsound, 0, 1, 1);
    SVARP(highlight, "");

    void toserver(char *text)
    {
        if(!joinred)
        {
            if(player1->state==CS_SPECTATOR)
            {
                conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f4: %s",
                    m_check(game::gamemode, M_TEAM) ? "\fY>\f1" : "\fX>\f0",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
            }
            else
            {
                conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f7: %s",
                    m_check(game::gamemode, M_TEAM) ? "\fY>\f1" : "\fX>\f0",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
                particle_textcopy(player1->abovehead(), text, PART_TEXT, 2000, 0xFFFFFF, 4.0f, -8);
            }
        }
        else
        {
            if(player1->state==CS_SPECTATOR)
            {
                conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f4: %s",
                    m_check(game::gamemode, M_TEAM) ? "\fZ>\f3" : "\fX>\f0",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
            }
            else
            {
                conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f7: %s",
                    m_check(game::gamemode, M_TEAM) ? "\fZ>\f3" : "\fX>\f0",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
                particle_textcopy(player1->abovehead(), text, PART_TEXT, 2000, 0xFFFFFF, 4.0f, -8);
            }
        }
        addmsg(N_TEXT, "rcs", player1, text);
        if(playchatsound)
        {
            playsound(S_CHAT);
            if(strstr(text, highlight)) playsound(S_HIGHLIGHT);
            if(strstr(text, "Hack") || strstr(text, "hack")) execute("sound (registersound \"../../faned/sounds/freesound.org/207779_2046066-lq\" 255)");
        }
        lua_N_TEXT(player1->clientnum, text);
    }
    COMMANDN(say, toserver, "C");

    void sayteam(char *text)
    {
        if(!joinred)
        {
            if(player1->state==CS_SPECTATOR)
            {
                conoutf(CON_CHAT, "\fY>\f1%s \fs\f4(%d%s)\fr\f4: %s",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
            }
            else
            {
                conoutf(CON_CHAT, "\fY>\f1%s \fs\f4(%d%s)\fr\f1: %s",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
                particle_textcopy(player1->abovehead(), text, PART_TEXT, 2000, 0x2222FF, 4.0f, -8);
            }
        }
        else
        {
            if(player1->state==CS_SPECTATOR)
            {
                conoutf(CON_CHAT, "\fZ>\f3%s \fs\f4(%d%s)\fr\f4: %s",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
            }
            else
            {
                conoutf(CON_CHAT, "\fZ>\f3%s \fs\f4(%d%s)\fr\f1: %s",
                    player1->name,
                    player1->clientnum,
                        isadmin(player1->clientnum) ?  ", Admin" :
                        (
                            isauth(player1->clientnum) ? ", Auth" :
                            (
                                ismaster(player1->clientnum) ? ", Master" : ""
                            )
                        ), text);
                particle_textcopy(player1->abovehead(), text, PART_TEXT, 2000, 0xFF2222, 4.0f, -8);
            }
        }
        addmsg(N_SAYTEAM, "rcs", player1, text);
        if(playchatsound)
        {
            playsound(S_CHAT);
            if(strstr(text, highlight)) playsound(S_HIGHLIGHT);
            if(strstr(text, "Hack") || strstr(text, "hack")) execute("sound (registersound \"../../faned/sounds/freesound.org/207779_2046066-lq\" 255)");
        }
        lua_N_SAYTEAM(player1->clientnum, text);
    }
    COMMAND(sayteam, "C");
    // End: Fanatic Edition

    ICOMMAND(servcmd, "C", (char *cmd), addmsg(N_SERVCMD, "rs", cmd));

    static void sendposition(fpsent *d, packetbuf &q)
    {
        putint(q, N_POS);
        putuint(q, d->clientnum);
        // 3 bits phys state, 1 bit life sequence, 2 bits move, 2 bits strafe
        uchar physstate = d->physstate | ((d->lifesequence&1)<<3) | ((d->move&3)<<4) | ((d->strafe&3)<<6);
        q.put(physstate);
        ivec o = ivec(vec(d->o.x, d->o.y, d->o.z-d->eyeheight).mul(DMF));
        uint vel = min(int(d->vel.magnitude()*DVELF), 0xFFFF), fall = min(int(d->falling.magnitude()*DVELF), 0xFFFF);
        // 3 bits position, 1 bit velocity, 3 bits falling, 1 bit material
        uint flags = 0;
        if(o.x < 0 || o.x > 0xFFFF) flags |= 1<<0;
        if(o.y < 0 || o.y > 0xFFFF) flags |= 1<<1;
        if(o.z < 0 || o.z > 0xFFFF) flags |= 1<<2;
        if(vel > 0xFF) flags |= 1<<3;
        if(fall > 0)
        {
            flags |= 1<<4;
            if(fall > 0xFF) flags |= 1<<5;
            if(d->falling.x || d->falling.y || d->falling.z > 0) flags |= 1<<6;
        }
        if((lookupmaterial(d->feetpos())&MATF_CLIP) == MAT_GAMECLIP) flags |= 1<<7;
        putuint(q, flags);
        loopk(3)
        {
            q.put(o[k]&0xFF);
            q.put((o[k]>>8)&0xFF);
            if(o[k] < 0 || o[k] > 0xFFFF) q.put((o[k]>>16)&0xFF);
        }
        uint dir = (d->yaw < 0 ? 360 + int(d->yaw)%360 : int(d->yaw)%360) + clamp(int(d->pitch+90), 0, 180)*360;
        q.put(dir&0xFF);
        q.put((dir>>8)&0xFF);
        q.put(clamp(int(d->roll+90), 0, 180));
        q.put(vel&0xFF);
        if(vel > 0xFF) q.put((vel>>8)&0xFF);
        float velyaw, velpitch;
        vectoyawpitch(d->vel, velyaw, velpitch);
        uint veldir = (velyaw < 0 ? 360 + int(velyaw)%360 : int(velyaw)%360) + clamp(int(velpitch+90), 0, 180)*360;
        q.put(veldir&0xFF);
        q.put((veldir>>8)&0xFF);
        if(fall > 0)
        {
            q.put(fall&0xFF);
            if(fall > 0xFF) q.put((fall>>8)&0xFF);
            if(d->falling.x || d->falling.y || d->falling.z > 0)
            {
                float fallyaw, fallpitch;
                vectoyawpitch(d->falling, fallyaw, fallpitch);
                uint falldir = (fallyaw < 0 ? 360 + int(fallyaw)%360 : int(fallyaw)%360) + clamp(int(fallpitch+90), 0, 180)*360;
                q.put(falldir&0xFF);
                q.put((falldir>>8)&0xFF);
            }
        }
    }

    void sendposition(fpsent *d, bool reliable)
    {
        if(d->state != CS_ALIVE && d->state != CS_EDITING) return;
        packetbuf q(100, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
        sendposition(d, q);
        sendclientpacket(q.finalize(), 0);
    }

    void sendpositions()
    {
        loopv(players)
        {
            fpsent *d = players[i];
            if((d == player1 || d->ai) && (d->state == CS_ALIVE || d->state == CS_EDITING))
            {
                packetbuf q(100);
                sendposition(d, q);
                for(int j = i+1; j < players.length(); j++)
                {
                    fpsent *d = players[j];
                    if((d == player1 || d->ai) && (d->state == CS_ALIVE || d->state == CS_EDITING))
                        sendposition(d, q);
                }
                sendclientpacket(q.finalize(), 0);
                break;
            }
        }
    }

    bool sendmessages(bool positionupdate) // Fanatic Edition
    {
        packetbuf p(MAXTRANS);
        if(sendcrc)
        {
            p.reliable();
            sendcrc = false;
            const char *mname = getclientmap();
            putint(p, N_MAPCRC);
            sendstring(mname, p);
            putint(p, mname[0] ? getmapcrc() : 0);
        }
        if(senditemstoserver)
        {
            if(!m_noitems || cmode!=NULL) p.reliable();
            if(!m_noitems) entities::putitems(p);
            if(cmode) cmode->senditems(p);
            senditemstoserver = false;
        }
        if(messages.length())
        {
            p.put(messages.getbuf(), messages.length());
            messages.setsize(0);
            if(messagereliable) p.reliable();
            messagereliable = false;
            messagecn = -1;
        }
        if(totalmillis-lastping>250)
        {
            putint(p, N_PING);
            putint(p, totalmillis);
            lastping = totalmillis;
        }
        sendclientpacket(p.finalize(), 1);
        return true; // Fanatic Edition
    }

    // Start: Fanatic Edition
    VARP(positionpacketdelay, 8, 33, 33);

    void c2sinfo(bool force)
    {
        static int lastupdate = -1000;
        bool positionupdate = totalmillis - lastupdate >= positionpacketdelay || force;
        if(positionupdate)
        {
            lastupdate = totalmillis;
            sendpositions();
        }
        if(sendmessages(positionupdate)) flushclient();
    }
    // End: Fanatic Edition

    void sendintro()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        putint(p, N_CONNECT);
        sendstring(player1->name, p);
        putint(p, player1->playermodel);
        string hash = "";
        if(connectpass[0])
        {
            server::hashpassword(player1->clientnum, sessionid, connectpass, hash);
            memset(connectpass, 0, sizeof(connectpass));
        }
        sendstring(hash, p);
        authkey *a = servauth[0] && autoauth ? findauthkey(servauth) : NULL;
        if(a)
        {
            a->lastauth = lastmillis;
            sendstring(a->desc, p);
            sendstring(a->name, p);
        }
        else
        {
            sendstring("", p);
            sendstring("", p);
        }
        sendclientpacket(p.finalize(), 1);
    }

    void updatepos(fpsent *d)
    {
        const float r = player1->radius+d->radius;
        const float dx = player1->o.x-d->o.x;
        const float dy = player1->o.y-d->o.y;
        const float dz = player1->o.z-d->o.z;
        const float rz = player1->aboveeye+d->eyeheight;
        const float fx = (float)fabs(dx), fy = (float)fabs(dy), fz = (float)fabs(dz);
        if(fx<r && fy<r && fz<rz && player1->state!=CS_SPECTATOR && d->state!=CS_DEAD)
        {
            if(fx<fy) d->o.y += dy<0 ? r-fy : -(r-fy);
            else      d->o.x += dx<0 ? r-fx : -(r-fx);
        }
        int lagtime = totalmillis-d->lastupdate;
        if(lagtime)
        {
            if(d->state!=CS_SPAWNING && d->lastupdate) d->plag = (d->plag*5+lagtime)/6;
            d->lastupdate = totalmillis;
        }
    }

    void parsepositions(ucharbuf &p)
    {
        int type;
        while(p.remaining()) switch(type = getint(p))
        {
            case N_DEMOPACKET: break;
            case N_POS:
            {
                int cn = getuint(p), physstate = p.get(), flags = getuint(p);
                vec o, vel, falling;
                float yaw, pitch, roll;
                loopk(3)
                {
                    int n = p.get(); n |= p.get()<<8; if(flags&(1<<k)) { n |= p.get()<<16; if(n&0x800000) n |= -1<<24; }
                    o[k] = n/DMF;
                }
                int dir = p.get(); dir |= p.get()<<8;
                yaw = dir%360;
                pitch = clamp(dir/360, 0, 180)-90;
                roll = clamp(int(p.get()), 0, 180)-90;
                int mag = p.get(); if(flags&(1<<3)) mag |= p.get()<<8;
                dir = p.get(); dir |= p.get()<<8;
                vecfromyawpitch(dir%360, clamp(dir/360, 0, 180)-90, 1, 0, vel);
                vel.mul(mag/DVELF);
                if(flags&(1<<4))
                {
                    mag = p.get(); if(flags&(1<<5)) mag |= p.get()<<8;
                    if(flags&(1<<6))
                    {
                        dir = p.get(); dir |= p.get()<<8;
                        vecfromyawpitch(dir%360, clamp(dir/360, 0, 180)-90, 1, 0, falling);
                    }
                    else falling = vec(0, 0, -1);
                    falling.mul(mag/DVELF);
                }
                else falling = vec(0, 0, 0);
                int seqcolor = (physstate>>3)&1;
                fpsent *d = getclient(cn);

                // Start: Fanatic Edition
                vec dir1;
                if(d != player1 && debugaim)
                {
                    float a1, a2;
                    vec to0, dir0, dir1, distv, from;
                    vecfromyawpitch(yaw, pitch, 1, 0, dir1);
                    to0 = vec(player1->o);
                    from = vec(d->o);
                    loopk(3) dir0[k]=to0[k]-from[k];

                    a1 = 0;
                    loopk(3) a1+=dir1[k]*dir1[k];
                    a2 = 0;
                    loopk(3) a2+=dir1[k]*dir0[k];
                    loopk(3) distv[k] = dir0[k]-a2/a1 * dir1[k];
                    a1 = 0;
                    loopk(3) a1+=distv[k]*distv[k];
                    a1 = sqrt(a1);

                    if((a1 < 30) && (a2>0)) conoutf(CON_DEBUG, "\f9FanEd\f7::debugaim: %s \f4(%d)\f7 aimed at you; miss distance \f4%f", d->name, d->clientnum, a1);
                }
                // End: Fanatic Edition

                if(!d || d->lifesequence < 0 || seqcolor != (d->lifesequence&1) || d->state == CS_DEAD) continue;
                float oldyaw = d->yaw, oldpitch = d->pitch, oldroll = d->roll;
                d->yaw = yaw;
                d->pitch = pitch;
                d->roll = roll;
                d->move = (physstate>>4)&2 ? -1 : (physstate>>4)&1;
                d->strafe = (physstate>>6)&2 ? -1 : (physstate>>6)&1;
                vec oldpos(d->o);
                if(allowmove(d))
                {
                    d->o = o;
                    d->o.z += d->eyeheight;
                    d->vel = vel;
                    d->falling = falling;
                    d->physstate = physstate&7;
                }
                updatephysstate(d);
                updatepos(d);
                if(smoothmove && d->smoothmillis>=0 && oldpos.dist(d->o) < smoothdist)
                {
                    d->newpos = d->o;
                    d->newyaw = d->yaw;
                    d->newpitch = d->pitch;
                    d->newroll = d->roll;
                    d->o = oldpos;
                    d->yaw = oldyaw;
                    d->pitch = oldpitch;
                    d->roll = oldroll;
                    (d->deltapos = oldpos).sub(d->newpos);
                    d->deltayaw = oldyaw - d->newyaw;
                    if(d->deltayaw > 180) d->deltayaw -= 360;
                    else if(d->deltayaw < -180) d->deltayaw += 360;
                    d->deltapitch = oldpitch - d->newpitch;
                    d->deltaroll = oldroll - d->newroll;
                    d->smoothmillis = lastmillis;
                }
                else d->smoothmillis = 0;
                if(d->state==CS_LAGGED || d->state==CS_SPAWNING) d->state = CS_ALIVE;
                break;
            }

            case N_TELEPORT:
            {
                int cn = getint(p), tp = getint(p), td = getint(p);
                fpsent *d = getclient(cn);
                if(!d || d->lifesequence < 0 || d->state==CS_DEAD) continue;
                // Start: Fanatic Edition
                if(identexists("onteleport"))
                {
                    defformatstring(str)("onteleport %d %d %d", d->clientnum, tp, td);
                    execute(str);
                }
                int it = (int)N_TELEPORT;
                lua_N_EVENT(it, cn, tp, td, 0);
                // End: Fanatic Edition
                entities::teleporteffects(d, tp, td, false);
                break;
            }

            case N_JUMPPAD:
            {
                int cn = getint(p), jp = getint(p);
                fpsent *d = getclient(cn);
                if(!d || d->lifesequence < 0 || d->state==CS_DEAD) continue;
                // Start: Fanatic Edition
                if(identexists("onjumppad"))
                {
                    defformatstring(str)("onjumppad %d %d", d->clientnum, jp);
                    execute(str);
                }
                int it = (int)N_JUMPPAD;
                lua_N_EVENT(it, cn, jp, 0, 0);
                // End: Fanatic Edition
                entities::jumppadeffects(d, jp, false);
                break;
            }

            default:
                neterr("type");
                return;
        }
    }

    void parsestate(fpsent *d, ucharbuf &p, bool resume = false)
    {
        if(!d) { static fpsent dummy; d = &dummy; }
        if(resume)
        {
            if(d==player1) getint(p);
            else d->state = getint(p);
            d->frags = getint(p);
            d->flags = getint(p);
            if(d==player1) getint(p);
            else d->quadmillis = getint(p);
        }
        d->lifesequence = getint(p);
        d->health = getint(p);
        d->maxhealth = getint(p);
        d->armour = getint(p);
        d->armourtype = getint(p);
        if(resume && d==player1)
        {
            getint(p);
            loopi(GUN_PISTOL-GUN_SG+1) getint(p);
        }
        else
        {
            int gun = getint(p);
            d->gunselect = clamp(gun, int(GUN_FIST), int(GUN_PISTOL));
            loopi(GUN_PISTOL-GUN_SG+1) d->ammo[GUN_SG+i] = getint(p);
        }
    }

    extern int deathscore;

    // Start: Fanatic Edition

    VARP(anticheat, 0, 1, 1);

    VARP(cubelimit, 0, 0, INT_MAX);

    int fragrec = 0;
    int fragrecid;
    stream *fragrecfile;

    ICOMMAND(fragrecstart, "", (), {
         conoutf("\f9FanEd\f7::fragrec: recording activated");
         fragrec = 1;
         string fname; formatstring(fname)("faned_fragrec.log");
         if(fragrecfile) delete fragrecfile;
         fragrecfile = openfile(fname, "w");
         fragrecid = 0;
         });

    ICOMMAND(fragrecstop, "", (), {
         conoutf("\f9FanEd\f7::fragrec: recording deactivated");
         fragrec = 0;
         if(fragrecfile) delete fragrecfile;
         });

    VARP(receiveai,      0, 1, 1);
    VARP(receivenewmap,  0, 1, 1);
    VARP(receiveremip,   0, 0, 1);
    VARP(receivesendmap, 0, 1, 1);
    // End: Fanatic Edition

    void parsemessages(int cn, fpsent *d, ucharbuf &p)
    {
        static char text[MAXTRANS];
        int type;
        bool mapchanged = false, demopacket = false;

        while(p.remaining()) switch(type = getint(p))
        {
            case N_DEMOPACKET: demopacket = true; break;

            case N_SERVINFO:
            {
                int mycn = getint(p), prot = getint(p);
                if(prot!=PROTOCOL_VERSION)
                {
                    conoutf(CON_ERROR, "you are using a different game protocol (you: %d, server: %d)", PROTOCOL_VERSION, prot);
                    playsound(S_ERROR); // Fanatic Edition
                    disconnect();
                    return;
                }
                sessionid = getint(p);
                player1->clientnum = mycn;
                if(getint(p) > 0) conoutf("this server is password protected");
                getstring(servinfo, p, sizeof(servinfo));
                getstring(servauth, p, sizeof(servauth));
                sendintro();
                break;
            }

            case N_WELCOME:
            {
                connected = true;
                notifywelcome();
                break;
            }

            case N_PAUSEGAME:
            {
                bool val = getint(p) > 0;
                int cn = getint(p);
                fpsent *a = cn >= 0 ? getclient(cn) : NULL;
                if(!demopacket)
                {
                    gamepaused = val;
                    player1->attacking = false;
                }
                if(a) conoutf("%s %s the game", colorname(a), val ? "paused" : "resumed"); 
                else conoutf("game is %s", val ? "paused" : "resumed");
                break;
            }

            case N_GAMESPEED:
            {
                int val = clamp(getint(p), 10, 1000), cn = getint(p);
                fpsent *a = cn >= 0 ? getclient(cn) : NULL;
                if(!demopacket) gamespeed = val;
                extern int slowmosp;
                if(m_sp && slowmosp) break;
                if(a) conoutf("%s set gamespeed to %d", colorname(a), val);
                else conoutf("gamespeed is %d", val);
                break;
            }
                
            case N_CLIENT:
            {
                int cn = getint(p), len = getuint(p);
                ucharbuf q = p.subbuf(len);
                parsemessages(cn, getclient(cn), q);
                break;
            }

            case N_SOUND:
            {
                if(!d) return;
                int sound = getint(p);
                playsound(sound, &d->o);
                break;
            }

            case N_TEXT:
            {
                // Start: Fanatic Edition
                if(!d) return;
                getstring(text, p);
                filtertext(text, text);
                if(isignored(d->clientnum)) break;
                if(!joinred)
                {
                    if(d->state!=CS_SPECTATOR) particle_textcopy(d->abovehead(), text, PART_TEXT, 2000, 0xFFFFFF, 4.0f, -8);
                    if(d->state==CS_SPECTATOR) {
                        conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f4: %s",
                            m_check(game::gamemode, M_TEAM) ? (isteam(d->team, player1->team) ? "\f1" : "\f3") : "\f0",
                            d->name,
                            d->clientnum,
                            isadmin(d->clientnum) ?  ", Admin" :
                            (
                                isauth(d->clientnum) ? ", Auth" :
                                (
                                    ismaster(d->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                    else {
                        conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f7: %s",
                            m_check(game::gamemode, M_TEAM) ? (isteam(d->team, player1->team) ? "\f1" : "\f3") : "\f0",
                            d->name,
                            d->clientnum,
                            isadmin(d->clientnum) ?  ", Admin" :
                            (
                                isauth(d->clientnum) ? ", Auth" :
                                (
                                    ismaster(d->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                }
                else
                {
                    if(d->state!=CS_SPECTATOR) particle_textcopy(d->abovehead(), text, PART_TEXT, 2000, 0xFFFFFF, 4.0f, -8);
                    if(d->state==CS_SPECTATOR) {
                        conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f4: %s",
                            m_check(game::gamemode, M_TEAM) ? (isteam(d->team, player1->team) ? "\f3" : "\f1") : "\f0",
                            d->name,
                            d->clientnum,
                            isadmin(d->clientnum) ?  ", Admin" :
                            (
                                isauth(d->clientnum) ? ", Auth" :
                                (
                                    ismaster(d->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                    else {
                        conoutf(CON_CHAT, "%s%s \fs\f4(%d%s)\fr\f7: %s",
                            m_check(game::gamemode, M_TEAM) ? (isteam(d->team, player1->team) ? "\f3" : "\f1") : "\f0",
                            d->name,
                            d->clientnum,
                            isadmin(d->clientnum) ?  ", Admin" :
                            (
                                isauth(d->clientnum) ? ", Auth" :
                                (
                                    ismaster(d->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                }
                if(playchatsound)
                {
                    playsound(S_CHAT);
                    if(strstr(text, player1->name)) playsound(S_HIGHLIGHT);
                    if(strstr(text, "Hack") || strstr(text, "hack")) execute("sound (registersound \"../../faned/sounds/freesound.org/207779_2046066-lq\" 255)");
                }
                lua_N_TEXT(d->clientnum, text);
                // End: Fanatic Edition
                break;
            }

            case N_SAYTEAM:
            {
                // Start: Fanatic Edition
                int tcn = getint(p);
                fpsent *t = getclient(tcn);
                getstring(text, p);
                filtertext(text, text);
                if(!t || isignored(t->clientnum)) break;
                if(!joinred)
                {
                    if(t->state!=CS_SPECTATOR) particle_textcopy(t->abovehead(), text, PART_TEXT, 2000, 0x2222FF, 4.0f, -8);
                    if(t->state==CS_SPECTATOR) {
                        conoutf(CON_CHAT, "\f1%s \fs\f4(%d%s)\fr\f4: %s",
                            t->name,
                            t->clientnum,
                            isadmin(t->clientnum) ?  ", Admin" :
                            (
                                isauth(t->clientnum) ? ", Auth" :
                                (
                                    ismaster(t->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                    else {
                        conoutf(CON_CHAT, "\f1%s \fs\f4(%d%s)\fr\f1: %s",
                            t->name,
                            t->clientnum,
                            isadmin(t->clientnum) ?  ", Admin" :
                            (
                                isauth(t->clientnum) ? ", Auth" :
                                (
                                    ismaster(t->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                }
                else
                {
                    if(t->state!=CS_SPECTATOR) particle_textcopy(t->abovehead(), text, PART_TEXT, 2000, 0xFF2222, 4.0f, -8);
                    if(t->state==CS_SPECTATOR) {
                        conoutf(CON_CHAT, "\f3%s \fs\f4(%d%s)\fr\f4: %s",
                            t->name,
                            t->clientnum,
                            isadmin(t->clientnum) ?  ", Admin" :
                            (
                                isauth(t->clientnum) ? ", Auth" :
                                (
                                    ismaster(t->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                    else {
                        conoutf(CON_CHAT, "\f3%s \fs\f4(%d%s)\fr\f3: %s",
                            t->name,
                            t->clientnum,
                            isadmin(t->clientnum) ?  ", Admin" :
                            (
                                isauth(t->clientnum) ? ", Auth" :
                                (
                                    ismaster(t->clientnum) ? ", Master" : ""
                                )
                            ), text);
                    }
                }
                if(playchatsound)
                {
                    playsound(S_CHAT);
                    if(strstr(text, player1->name)) playsound(S_HIGHLIGHT);
                    if(strstr(text, "Hack") || strstr(text, "hack")) execute("sound (registersound \"../../faned/sounds/freesound.org/207779_2046066-lq\" 255)");
                }
                lua_N_SAYTEAM(t->clientnum, text);
                // End: Fanatic Edition
                break;
            }

            case N_MAPCHANGE:
                getstring(text, p);
                changemapserv(text, getint(p));
                mapchanged = true;
                if(getint(p)) entities::spawnitems();
                else senditemstoserver = false;
                if(identexists("onmapchange"))
                {
                    defformatstring(str)("onmapchange %s", text);
                    execute(str);
                }
                break;

            case N_FORCEDEATH:
            {
                int cn = getint(p);
                fpsent *d = cn==player1->clientnum ? player1 : newclient(cn);
                if(!d) break;
                if(d==player1)
                {
                    if(editmode) toggleedit();
                    stopfollowing();
                    if(deathscore) showscores(true);
                }
                else d->resetinterp();
                d->state = CS_DEAD;
                break;
            }

            case N_ITEMLIST:
            {
                int n;
                while((n = getint(p))>=0 && !p.overread())
                {
                    if(mapchanged) entities::setspawn(n, true);
                    getint(p);
                }
                break;
            }

            case N_INITCLIENT:
            {
                int cn = getint(p);
                fpsent *d = newclient(cn);
                if(!d)
                {
                    getstring(text, p);
                    getstring(text, p);
                    getint(p);
                    break;
                }
                getstring(text, p);
                filtertext(text, text, false, MAXNAMELEN);
                if(!text[0]) copystring(text, "unnamed");
                if(d->name[0])
                {
                    if(strcmp(d->name, text) && !isignored(d->clientnum))
                    {
                        conoutf("%s is now known as %s", colorname(d), colorname(d, text));
                    }
                }
                else
                {
                    conoutf("\f0connected:\f7 %s", colorname(d, text)); // Fanatic Edition
                    if(needclipboard >= 0) needclipboard++;
                }
                copystring(d->name, text, MAXNAMELEN+1);
                getstring(text, p);
                filtertext(d->team, text, false, MAXTEAMLEN);
                d->playermodel = getint(p);
                if(identexists("onconnect"))
                {
                    defformatstring(str)("onconnect %d %d", d->clientnum, d->playermodel);
                    execute(str);
                }
                break;
            }

            case N_SWITCHNAME:
                getstring(text, p);
                if(d)
                {
                    filtertext(text, text, false, MAXNAMELEN);
                    if(!text[0]) copystring(text, "unnamed");
                    if(strcmp(text, d->name))
                    {
                        if(!isignored(d->clientnum)) conoutf("%s is now known as %s", colorname(d), colorname(d, text));
                        copystring(d->name, text, MAXNAMELEN+1);
                    }
                }
                break;

            case N_SWITCHMODEL:
            {
                int model = getint(p);
                if(d)
                {
                    d->playermodel = model;
                    if(d->ragdoll) cleanragdoll(d);
                }
                break;
            }

            case N_CDIS:
                clientdisconnected(getint(p));
                break;

            case N_SPAWN:
            {
                if(d)
                {
                    if(d->state==CS_DEAD && d->lastpain) saveragdoll(d);
                    d->respawn();
                }
                parsestate(d, p);
                if(!d) break;
                d->state = CS_SPAWNING;
                if(player1->state==CS_SPECTATOR && following==d->clientnum)
                lasthit = 0;
                break;
            }

            case N_SPAWNSTATE:
            {
                int scn = getint(p);
                fpsent *s = getclient(scn);
                if(!s) { parsestate(NULL, p); break; }
                if(s->state==CS_DEAD && s->lastpain) saveragdoll(s);
                if(s==player1)
                {
                    if(editmode) toggleedit();
                    stopfollowing();
                }
                s->respawn();
                parsestate(s, p);
                s->state = CS_ALIVE;
                if(cmode) cmode->pickspawn(s);
                else findplayerspawn(s);
                if(s == player1)
                {
                    showscores(false);
                    lasthit = 0;
                }
                if(cmode) cmode->respawned(s);
                ai::spawned(s);
                addmsg(N_SPAWN, "rcii", s, s->lifesequence, s->gunselect);
                break;
            }

            case N_SHOTFX:
            {
                int scn = getint(p), gun = getint(p), id = getint(p);
                vec from, to;
                loopk(3) from[k] = getint(p)/DMF;
                loopk(3) to[k] = getint(p)/DMF;
                fpsent *s = getclient(scn);
                if(!s) break;

                // Start: Fanatic Edition
                lua_N_SHOTFX(scn, gun, &to);

                vec to0, dir, dir0, distv;
                float a1, a2;

                if(debugshoot)
                {
                    to0 = vec(player1->o);
                    loopk(3) dir0[k]=to0[k]-from[k];
                    loopk(3) dir[k]=to[k]-from[k];

                    a1 = 0;
                    loopk(3) a1+=dir[k]*dir[k];
                    a2 = 0;
                    loopk(3) a2+=dir[k]*dir0[k];
                    loopk(3) distv[k] = dir0[k]-a2/a1 * dir[k];
                    a1 = 0;
                    loopk(3) a1+=distv[k]*distv[k];
                    a1 = sqrt(a1);

                    if((a1 < 30) && (a2 > 0))
                    {
                        conoutf("CON_DEBUG, \f9FanEd\f7::debugshoot: %s \f4(%d)\f7 shot at you; distance \f4%f", s->name, s->clientnum, a1);
                    }
                }
                // End: Fanatic Edition

                if(gun>GUN_FIST && gun<=GUN_PISTOL && s->ammo[gun]) s->ammo[gun]--;
                s->gunselect = clamp(gun, (int)GUN_FIST, (int)GUN_PISTOL);
                s->gunwait = guns[s->gunselect].attackdelay;
                int prevaction = s->lastaction;
                s->lastaction = lastmillis;
                s->lastattackgun = s->gunselect;
                shoteffects(s->gunselect, from, to, s, false, id, prevaction);

                // Start: Fanatic Edition
                s->totalshots += guns[s->gunselect].damage*(s->quadmillis ? 4 : 1)*(s->gunselect==GUN_SG ? MAXRAYS : 1);
                if(anticheat && !intermission && !m_edit && from.dist(to) > guns[gun].range + 1)
                {
                    conoutf(CON_WARN,
                        "\f9FanEd\f7::anticheat: Client \f4%s (%d)\f7, Type: \f4from.dist(to) > guns[gun].range + 1\f7, AKA: \f4Modified Gunrange",
                        s->name,
                        s->clientnum
                    );
                    playsound(S_WARNING);
                }
                if(anticheat && !m_edit && (s->gunselect < GUN_FIST || s->gunselect > GUN_PISTOL))
                {
                    conoutf(CON_WARN,
                        "\f9FanEd\f7::anticheat: Client \f4%s (%d)\f7, Type: \f4from.dist(to) > guns[gun].range + 1\f7, AKA: \f4Unknown Weapon",
                        s->name,
                        s->clientnum
                    );
                    playsound(S_WARNING);
                }
                // End: Fanatic Edition
                break;
            }

            case N_EXPLODEFX:
            {
                int ecn = getint(p), gun = getint(p), id = getint(p);
                fpsent *e = getclient(ecn);
                if(!e) break;
                explodeeffects(gun, e, false, id);
                break;
            }
            case N_DAMAGE:
            {
                int tcn = getint(p),
                    acn = getint(p),
                    damage = getint(p),
                    armour = getint(p),
                    health = getint(p);
                fpsent *target = getclient(tcn),
                       *actor = getclient(acn);
                if(!target || !actor) break;
                target->armour = armour;
                target->health = health;
                if(target->state == CS_ALIVE && actor != player1) target->lastpain = lastmillis;
                damaged(damage, target, actor, false);
                // Start: Fanatic Edition
                if(!actor->skill && actor != player1) actor->totaldamage += damage;
                fpsent *d = getclient(acn);
                if(debugdamage && target == player1)
                {
                    conoutf(CON_DEBUG,
                        "\f9FanEd\f7::debugdamage: you've been damaged by %s \f4(%d)\f7; damage \f4%d",
                        d->name,
                        d->clientnum,
                        damage
                    );
                }
                if(identexists("ondamage"))
                {
                    defformatstring(str)("ondamage %d %d %d %d", actor->clientnum, target->clientnum, actor->gunselect, damage);
                    execute(str);
                }
                lua_N_DAMAGE(acn, tcn, damage, armour, health);
                // End: Fanatic Edition
                break;
            }

            case N_HITPUSH:
            {
                int tcn = getint(p), gun = getint(p), damage = getint(p);
                fpsent *target = getclient(tcn);
                vec dir;
                loopk(3) dir[k] = getint(p)/DNF;
                if(target) target->hitpush(damage * (target->health<=0 ? deadpush : 1), dir, NULL, gun);
                break;
            }

            case N_DIED:
            {
                int vcn = getint(p), acn = getint(p), frags = getint(p), tfrags = getint(p);
                fpsent *victim = getclient(vcn),
                       *actor = getclient(acn);
                if(!actor) break;
                actor->frags = frags;
                if(m_teammode) setteaminfo(actor->team, tfrags);
                if(actor!=player1 && (!cmode || !cmode->hidefrags()))
                {
                    defformatstring(ds)("%d", actor->frags);
                    particle_textcopy(actor->abovehead(), ds, PART_TEXT, 2000, 0x32FF64, 4.0f, -8);
                }
                if(!victim) break;
                // Start: Fanatic Edition
                if(fragrec)
                {
                    string fraginfo;
                    formatstring(fraginfo)("%d %d %d %d %d %d %d %d %d %d\n",
                        victim->clientnum,
                        actor->clientnum,
                        victim->gunselect,
                        actor->gunselect,
                        int(victim->o.x),
                        int(victim->o.y),
                        int(victim->o.z),
                        int(actor->o.x),
                        int(actor->o.y),
                        int(actor->o.z));
                    fragrecid++;
                    fragrecfile->printf("%s", fraginfo);
                    conoutf("\f9FanEd\f7::fragrec: id \f9#%d\f7 logged", fragrecid);
                }
                if(identexists("ondied"))
                {
                    defformatstring(str)("ondied %d %d %d", actor->clientnum, victim->clientnum, actor->gunselect);
                    execute(str);
                }
                lua_N_DIED(acn, vcn, frags, tfrags);
                // End: Fanatic Edition
                killed(victim, actor);
                break;
            }

            case N_TEAMINFO:
                for(;;)
                {
                    getstring(text, p);
                    if(p.overread() || !text[0]) break;
                    int frags = getint(p);
                    if(p.overread()) break;
                    if(m_teammode) setteaminfo(text, frags);
                }
                break;

            case N_GUNSELECT:
            {
                if(!d) return;
                int gun = getint(p);
                d->gunselect = clamp(gun, int(GUN_FIST), int(GUN_PISTOL));
                playsound(S_WEAPLOAD, &d->o);
                // Start: Fanatic Edition
                int it = (int)N_GUNSELECT;
                lua_N_EVENT(it, gun, d->clientnum, 0, 0);
                // End: Fanatic Edition
                break;
            }

            case N_TAUNT:
            {
                if(!d) return;
                d->lasttaunt = lastmillis;
                break;
            }

            case N_RESUME:
            {
                for(;;)
                {
                    int cn = getint(p);
                    if(p.overread() || cn<0) break;
                    fpsent *d = (cn == player1->clientnum ? player1 : newclient(cn));
                    parsestate(d, p, true);
                }
                break;
            }

            case N_ITEMSPAWN:
            {
                int i = getint(p);
                if(!entities::ents.inrange(i)) break;
                entities::setspawn(i, true);
                ai::itemspawned(i);
                playsound(S_ITEMSPAWN, &entities::ents[i]->o, NULL, 0, 0, 0, -1, 0, 1500);
                // Start: Fanatic Edition
                // #if 0
                // const char *name = entities::itemname(i);
                // if(name) particle_text(entities::ents[i]->o, name, PART_TEXT, 2000, 0x32FF64, 4.0f, -8);
                // #endif
                // int icon = entities::itemicon(i);
                // if(icon >= 0) particle_icon(vec(0.0f, 0.0f, 4.0f).add(entities::ents[i]->o), icon%4, icon/4, PART_HUD_ICON, 2000, 0xFFFFFF, 2.0f, -8);
                // End: Fanatic Edition
                break;
            }

            case N_ITEMACC:
            {
                int i = getint(p), cn = getint(p);
                fpsent *d = getclient(cn);
                entities::pickupeffects(i, d);
                break;
            }

            case N_CLIPBOARD:
            {
                int cn = getint(p), unpacklen = getint(p), packlen = getint(p);
                fpsent *d = getclient(cn);
                ucharbuf q = p.subbuf(max(packlen, 0));
                if(d) unpackeditinfo(d->edit, q.buf, q.maxlen, unpacklen);
                break;
            }

            case N_EDITF:
            case N_EDITT:
            case N_EDITM:
            case N_FLIP:
            case N_COPY:
            case N_PASTE:
            case N_ROTATE:
            case N_REPLACE:
            case N_DELCUBE:
            {
                if(!d || iseditmuted(d->clientnum)) return; // Fanatic Edition
                selinfo sel;
                sel.o.x = getint(p); sel.o.y = getint(p); sel.o.z = getint(p);
                sel.s.x = getint(p); sel.s.y = getint(p); sel.s.z = getint(p);
                sel.grid = getint(p); sel.orient = getint(p);
                sel.cx = getint(p); sel.cxs = getint(p); sel.cy = getint(p), sel.cys = getint(p);
                sel.corner = getint(p);
                if(anticheat && cubelimit > 0 && sel.s.x*sel.s.y*sel.s.z > cubelimit)
                {
                    conoutf(CON_WARN,
                       "\f9FanEd\f7::anticheat: Client \f4%s (%d)\f7, Cubes \f4%d\f7, Type: \f4cubelimit > 0 && sel.s.x*sel.s.y*sel.s.z > cubelimit",
                       d->name,
                       d->clientnum,
                       sel.s.x*sel.s.y*sel.s.z
                    );
                    playsound(S_WARNING);
                }
                int dir, mode, tex, newtex, mat, filter, allfaces, insel;
                ivec moveo;
                switch(type)
                {
                    case N_EDITF: dir = getint(p); mode = getint(p); if(sel.validate()) mpeditface(dir, mode, sel, false); break;
                    case N_EDITT: tex = getint(p); allfaces = getint(p); if(sel.validate()) mpedittex(tex, allfaces, sel, false); break;
                    case N_EDITM: mat = getint(p); filter = getint(p); if(sel.validate()) mpeditmat(mat, filter, sel, false); break;
                    case N_FLIP: if(sel.validate()) mpflip(sel, false); break;
                    case N_COPY: if(d && sel.validate()) mpcopy(d->edit, sel, false); break;
                    case N_PASTE: if(d && sel.validate()) mppaste(d->edit, sel, false); break;
                    case N_ROTATE: dir = getint(p); if(sel.validate()) mprotate(dir, sel, false); break;
                    case N_REPLACE: tex = getint(p); newtex = getint(p); insel = getint(p); if(sel.validate()) mpreplacetex(tex, newtex, insel>0, sel, false); break;
                    case N_DELCUBE: if(sel.validate()) mpdelcube(sel, false); break;
                }
                break;
            }
            case N_REMIP:
            {
                // Start: Fanatic Edition
                if(!d) return;

                if(!receiveremip || iseditmuted(d->clientnum))
                {
                    conoutf(CON_INFO, "\f9FanEd\f7::parsemessages: N_REMIP by %s \f4(%d)\f7 was ignored", d->name, d->clientnum);
                    break;
                }
                else
                {
                    conoutf(CON_INFO, "\f9FanEd\f7::parsemessages: %s \f4(%d)\f7 remipped", d->name, d->clientnum);
                    mpremip(false);
                }
                break;
                // End: Fanatic Edition
            }
            case N_EDITENT:
            {
                if(!d || iseditmuted(d->clientnum)) return; // Fanatic Edition
                int i = getint(p);
                float x = getint(p)/DMF, y = getint(p)/DMF, z = getint(p)/DMF;
                int type = getint(p);
                int attr1 = getint(p), attr2 = getint(p), attr3 = getint(p), attr4 = getint(p), attr5 = getint(p);

                mpeditent(i, vec(x, y, z), type, attr1, attr2, attr3, attr4, attr5, false);
                break;
            }
            case N_EDITVAR:
            {
                if(!d || iseditmuted(d->clientnum)) return; // Fanatic Edition
                int type = getint(p);
                getstring(text, p);
                string name;
                filtertext(name, text, false, MAXSTRLEN-1);
                ident *id = getident(name);
                switch(type)
                {
                    case ID_VAR:
                    {
                        int val = getint(p);
                        if(id && id->flags&IDF_OVERRIDE && !(id->flags&IDF_READONLY)) setvar(name, val);
                        break;
                    }
                    case ID_FVAR:
                    {
                        float val = getfloat(p);
                        if(id && id->flags&IDF_OVERRIDE && !(id->flags&IDF_READONLY)) setfvar(name, val);
                        break;
                    }
                    case ID_SVAR:
                    {
                        getstring(text, p);
                        if(id && id->flags&IDF_OVERRIDE && !(id->flags&IDF_READONLY)) setsvar(name, text);
                        break;
                    }
                }
                printvar(d, id);
                break;
            }

            case N_PONG:
                addmsg(N_CLIENTPING, "i", player1->ping = (player1->ping*5+totalmillis-getint(p))/6);
                break;

            case N_CLIENTPING:
                if(!d) return;
                d->ping = getint(p);
                break;

            case N_TIMEUP:
                timeupdate(getint(p));
                break;

            case N_SERVMSG:
                // Start: Fanatic Edition
                getstring(text, p);
                lua_N_SERVCMD(-1, text);
                conoutf("%s", text);
                // End: Fanatic Edition
                break;

            case N_SENDDEMOLIST:
            {
                int demos = getint(p);
                if(demos <= 0) conoutf(CON_INFO, "\f9FanEd\f7::parsemessages: no demos available"); // Fanatic Edition
                else loopi(demos)
                {
                    getstring(text, p);
                    if(p.overread()) break;
                    conoutf(CON_INFO, "\f9FanEd\f7::parsemessages: demo \f9#%d\f7. \f4%s", i+1, text); // Fanatic Edition
                }
                break;
            }

            case N_DEMOPLAYBACK:
            {
                int on = getint(p);
                if(on) player1->state = CS_SPECTATOR;
                else clearclients();
                demoplayback = on!=0;
                player1->clientnum = getint(p);
                gamepaused = false;
                const char *alias = on ? "demostart" : "demoend";
                if(identexists(alias)) execute(alias);
                break;
            }

            case N_CURRENTMASTER:
            {
                int mm = getint(p), mn;
                loopv(players) players[i]->privilege = PRIV_NONE;
                while((mn = getint(p))>=0 && !p.overread())
                {
                    fpsent *m = mn==player1->clientnum ? player1 : newclient(mn);
                    int priv = getint(p);
                    if(m) m->privilege = priv;
                }
                if(mm != mastermode)
                {
                    mastermode = mm;
                    conoutf(CON_INFO, "mastermode is %s \f4(%d)", server::mastermodename(mastermode), mastermode); // Fanatic Edition
                }
                break;
            }

            case N_MASTERMODE:
            {
                mastermode = getint(p);
                conoutf(CON_INFO, "mastermode is %s \f4(%d)", server::mastermodename(mastermode), mastermode); // Fanatic Edition
                break;
            }

            case N_EDITMODE:
            {
                // Start: Fanatic Edition
                if(anticheat && !m_edit)
                {
                    conoutf(CON_WARN,
                       "\f9FanEd\f7::anticheat: Client \f4%s (%d)\f7, Type: \f4N_EDITMODE\f7, AKA: \f4Edittoggle in non edit game mode",
                       d->name, d->clientnum);
                    playsound(S_WARNING);
                }
                int val = getint(p);
                if(!d) break;
                if(val)
                {
                    d->editstate = d->state;
                    d->state = CS_EDITING;
                }
                else
                {
                    d->state = d->editstate;
                    if(d->state==CS_DEAD) deathstate(d, true);
                }
                lua_N_EDITMODE(d->clientnum, val);
                // End: Fanatic Edition
                break;
            }

            case N_SPECTATOR:
            {
                int sn = getint(p), val = getint(p);
                fpsent *s;
                if(sn==player1->clientnum)
                {
                    s = player1;
                    if(val && remote && !player1->privilege) senditemstoserver = false;
                }
                else s = newclient(sn);
                if(!s) return;
                // Start: Fanatic Edition
                if(mastermode != MM_LOCKED)
                {
                    if(val) conoutf(CON_INFO, "%s switched to spectators", colorname(s));
                    else conoutf(CON_INFO, "%s switched to team %s", colorname(s), s->team);
                }
                // End: Fanatic Edition
                if(val)
                {
                    if(s==player1)
                    {
                        if(editmode) toggleedit();
                        if(s->state==CS_DEAD) showscores(false);
                        disablezoom();
                    }
                    s->state = CS_SPECTATOR;
                }
                else if(s->state==CS_SPECTATOR)
                {
                    if(s==player1) stopfollowing();
                    deathstate(s, true);
                }
                break;
            }

            case N_SETTEAM:
            {
                int wn = getint(p);
                getstring(text, p);
                int reason = getint(p);
                fpsent *w = getclient(wn);
                if(!w) return;
                filtertext(w->team, text, false, MAXTEAMLEN);
                static const char * const fmt[2] = { "%s switched to team %s", "%s forced to team %s"};
                if(reason >= 0 && size_t(reason) < sizeof(fmt)/sizeof(fmt[0]))
                    conoutf(fmt[reason], colorname(w), w->team);
                break;
            }

            #define PARSEMESSAGES 1
            #include "capture.h"
            #include "ctf.h"
            #include "collect.h"
            #undef PARSEMESSAGES

            case N_ANNOUNCE:
            {
                int t = getint(p);
                if     (t==I_QUAD)  { playsound(S_V_QUAD10, NULL, NULL, 0, 0, 0, -1, 0, 3000);  conoutf(CON_GAMEINFO, "\f2quad damage will spawn in 10 seconds!"); }
                else if(t==I_BOOST) { playsound(S_V_BOOST10, NULL, NULL, 0, 0, 0, -1, 0, 3000); conoutf(CON_GAMEINFO, "\f2+10 health will spawn in 10 seconds!"); }
                break;
            }

            case N_NEWMAP:
            {
                // Start: Fanatic Edition
                int size = getint(p);
                if(!receivenewmap || iseditmuted(d->clientnum))
                {
                    conoutf(CON_INFO, "\f9FanEd\f7::parsemessages: N_NEWMAP by %s \f4(%d)\f7 was ignored", d->name, d->clientnum);
                    break;
                }
                if(size>=0) emptymap(size, true, NULL);
                else enlargemap(true);
                if(d && d!=player1)
                {
                    int newsize = 0;
                    while(1<<newsize < getworldsize()) newsize++;
                    conoutf(size>=0 ? "\f9FanEd\f7::parsemessages: %s \f4(%d)\f7 started a new map of size %d" : "\f9FanEd\f7::parsemessages: %s \f4(%d)\f7 enlarged the map to size %d", d->name, d->clientnum, newsize);
                }
                // End: Fanatic Edition
                break;
            }

            case N_REQAUTH:
            {
                getstring(text, p);
                if(autoauth && text[0] && tryauth(text)) conoutf("server requested authkey \"%s\"", text);
                break;
            }

            case N_AUTHCHAL:
            {
                getstring(text, p);
                authkey *a = findauthkey(text);
                uint id = (uint)getint(p);
                getstring(text, p);
                if(a && a->lastauth && lastmillis - a->lastauth < 60*1000)
                {
                    vector<char> buf;
                    answerchallenge(a->key, text, buf);
                    // conoutf(CON_DEBUG, "answering %u, challenge %s with %s", id, text, buf.getbuf());
                    packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
                    putint(p, N_AUTHANS);
                    sendstring(a->desc, p);
                    putint(p, id);
                    sendstring(buf.getbuf(), p);
                    sendclientpacket(p.finalize(), 1);
                }
                break;
            }

            case N_INITAI:
            {
                int bn = getint(p), on = getint(p), at = getint(p), sk = clamp(getint(p), 1, 101), pm = getint(p);
                string name, team;
                getstring(text, p);
                filtertext(name, text, false, MAXNAMELEN);
                getstring(text, p);
                filtertext(team, text, false, MAXTEAMLEN);
                fpsent *b = newclient(bn);
                if(!b) break;
                // Start: Fanatic Edition
                if(!receiveai)
                {
                    conoutf(CON_INFO, "\f9FanEd\f7::parsemessages: N_INITAI %s \fs\f4[%d]\fr (Skill: %d) by cn %d was ignored", name, bn, sk, on);
                    break;
                }
                // End: Fanatic Edition
                ai::init(b, at, on, sk, bn, pm, name, team);
                break;
            }

            case N_SERVCMD:
                getstring(text, p);
                break;

            default:
                neterr("type", cn < 0);
                conoutf(CON_ERROR, "\f9FanEd\f7::neterr: \f3type %d", cn < 0);
                return;
        }
    }

    // Start: Fanatic Edition
    bool _getfile = false;
    string filename;

    void receivefile(packetbuf &p)
    {
        int type;
        while(p.remaining()) switch(type = getint(p))
        {
            case N_DEMOPACKET: return;
            case N_SENDDEMO:
            {
                string curtime;
                time_t t = time(NULL);
                size_t len = strftime(curtime, sizeof(curtime), "demo_%Y-%m-%d_%H.%M.%S", localtime(&t));
                curtime[min(len, sizeof(curtime)-1)] = '\0';
                defformatstring(fname)("demos/%s.dmo", curtime);
                stream *demo = openrawfile(fname, "wb");
                if(!demo) return;
                conoutf(CON_INFO, "\f9FanEd\f7::receivefile: received demo \f4\"%s\"", fname);
                ucharbuf b = p.subbuf(p.remaining());
                demo->write(b.buf, b.maxlen);
                delete demo;
                break;
            }

            case N_SENDMAP:
            {
                if(!receivesendmap)
                {
                        conoutf(CON_INFO, "\f9FanEd\f7::receivefile: N_SENDMAP was ignored");
                        return;
                }
                if(!m_edit) return;
                string oldname;
                copystring(oldname, getclientmap());
                if(!_getfile)
                {
                    defformatstring(mname)("getmap_%d", lastmillis);
                    defformatstring(fname)("packages/base/%s.ogz", mname);
                    stream *map = openrawfile(path(fname), "wb");
                    if(!map) return;
                    ucharbuf b = p.subbuf(p.remaining());
                    map->write(b.buf, b.maxlen);
                    conoutf(CON_INFO, "\f9FanEd\f7::receivefile: received map \f4(%4.3f KB)", (float(map->size())/1024.f));
                    delete map;
                    if(load_world(mname, oldname[0] ? oldname : NULL))
                        entities::spawnitems(true);
                    remove(findfile(fname, "rb"));
                    break;
                }
                else
                {
                    _getfile = false;
                    defformatstring(mname)("%s", filename);
                    defformatstring(fname)("%s", mname);
                    stream *map = openrawfile(path(fname), "wb");
                    if(!map) return;
                    conoutf(CON_INFO, "\f9FanEd\f7::receivefile: received file \f4\"%s\"", filename);
                    ucharbuf b = p.subbuf(p.remaining());
                    map->write(b.buf, b.maxlen);
                    delete map;
                    break;
                }
            }
        }
    }
    // End: Fanatic Edition

    void parsepacketclient(int chan, packetbuf &p)
    {
        if(p.packet->flags&ENET_PACKET_FLAG_UNSEQUENCED) return;
        switch(chan)
        {
            case 0:
                parsepositions(p);
                break;

            case 1:
                parsemessages(-1, NULL, p);
                break;

            case 2:
                receivefile(p);
                break;
        }
    }

    // Start: Fanatic Edition
    void getmap()
    {
        if(!m_edit)
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::getmap: \f3#error: \f7\"getmap\" only works in coop edit mode");
            playsound(S_ERROR);
            return;
        }
        conoutf(CON_INFO, "\f9FanEd\f7::getmap: getting map...");
        _getfile = false;
        addmsg(N_GETMAP, "r");
    }
    COMMAND(getmap, "");
    
    void getfile(char *file)
    {
        if(!m_edit)
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::getfile: \f3#error: \f7\"getfile\" only works in coop edit mode");
            playsound(S_ERROR);
            return;
        }
        formatstring(filename)("%s", file);
        if(!file[0]) formatstring(filename)("getfile_%d", lastmillis);
        conoutf(CON_INFO, "\f9FanEd\f7::getfile: getting file...");
        _getfile = true;
        addmsg(N_GETMAP, "r");
    }
    ICOMMAND(getfile, "s", (char *file), getfile(file));
    // End: Fanatic Edition

    void stopdemo()
    {
        if(remote)
        {
            if(player1->privilege<PRIV_MASTER) return;
            addmsg(N_STOPDEMO, "r");
        }
        else server::stopdemo();
    }
    COMMAND(stopdemo, "");

    void recorddemo(int val)
    {
        if(remote && player1->privilege<PRIV_MASTER) return;
        addmsg(N_RECORDDEMO, "ri", val);
    }
    ICOMMAND(recorddemo, "i", (int *val), recorddemo(*val));

    void cleardemos(int val)
    {
        if(remote && player1->privilege<PRIV_MASTER) return;
        addmsg(N_CLEARDEMOS, "ri", val);
    }
    ICOMMAND(cleardemos, "i", (int *val), cleardemos(*val));

    // Start: Fanatic Edition
    void getdemo(int i)
    {
        if(i<=0) conoutf(CON_INFO, "\f9FanEd\f7::getdemo: getting demo...");
        else conoutf(CON_INFO, "\f9FanEd\f7::getdemo: getting demo \f4%d\f7...", i);
        addmsg(N_GETDEMO, "ri", i);
    }
    ICOMMAND(getdemo, "i", (int *val), getdemo(*val));
    // End: Fanatic Edition

    void listdemos()
    {
        conoutf("listing demos...");
        addmsg(N_LISTDEMOS, "r");
    }
    COMMAND(listdemos, "");

    // Start: Fanatic Edition
    void sendmap(bool full)
    {
        if(!m_edit || (player1->state==CS_SPECTATOR && remote && !player1->privilege))
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::sendmap: \f3#error: \f7\"sendmap\" only works in coop edit mode");
            playsound(S_ERROR);
            return;
        }
        defformatstring(mname)("sendmap_%d", lastmillis);
        save_world(mname, full);
        defformatstring(fname)("packages/base/%s.ogz", mname);
        stream *map = openrawfile(path(fname), "rb");
        if(map)
        {
            stream::offset len = map->size();
            if(len > 32*1024*1024)
            {
                conoutf(CON_ERROR, "\f9FanEd\f7::sendmap: \f3#error: \f7map is too large");
                playsound(S_ERROR);
            }
            else if(len <= 0)
            {
                conoutf(CON_ERROR, "\f9FanEd\f7::sendfile: \f3#error: \f7could not read map");
                playsound(S_ERROR);
            }
            else
            {
                conoutf("\f9FanEd\f7::sendmap: sending map \f4(%4.3f KB)...", (float(map->size())/1024.f));
                sendfile(-1, 2, map);
                if(needclipboard >= 0) needclipboard++;
            }
            delete map;
        }
        else
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::sendfile: \f3#error: \f7could not read map");
            playsound(S_ERROR);
        }
        remove(findfile(fname, "rb"));
    }
    ICOMMAND(sendmap, "", (), sendmap(true));
    ICOMMAND(sendmapfull, "", (), sendmap(false));

    void sendfile(char *file)
    {
        if(!m_edit || (player1->state==CS_SPECTATOR && remote && !player1->privilege))
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::sendfile: \f3#error: \f7\"sendfile\" only works in coop edit mode");
            playsound(S_ERROR);
            return;
        }
        defformatstring(fname)("%s", file);
        if(!file[0])
        {
            conoutf(CON_WARN, "\f9FanEd\f7::sendfile: \f6#warning:\f7 no file to send");
            return;
        }
        stream *map = openrawfile(path(fname), "rb");
        if(map)
        {
            stream::offset len = map->size();
            if(len > 32*1024*1024)
            {
                conoutf(CON_ERROR, "\f9FanEd\f7::sendfile: \f3#error: \f7file is too large"); 
                playsound(S_ERROR);
            }
            else if(len <= 0)
            {
                conoutf(CON_ERROR, "\f9FanEd\f7::sendfile: \f3#error: \f7could not read file");
                playsound(S_ERROR);
            }
            else
            {
                conoutf("\f9FanEd\f7::sendfile: sending file \f4\"%s\"\f7...", file);
                sendfile(-1, 2, map);
                if(needclipboard >= 0) needclipboard++;
            }
            delete map;
        }
        else
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::sendfile: \f3#error: \f7could not read file");
            playsound(S_ERROR);
        }
    }
    ICOMMAND(sendfile, "s", (char *file), sendfile(file));

    VARP(gotodist, INT_MIN, -32, INT_MAX);

    void gotoplayer(const char *arg)
    {
        if(!m_edit)
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::gotoplayer: \f3#error: \f7\"goto\" only works in coop edit mode");
            playsound(S_ERROR);
            return;
        }
        int i = parseplayer(arg);
        if(i>=0)
        {
            fpsent *d = getclient(i);
            if(!d || d==player1) return;
            player1->o = d->o;
            vec dir;
            vecfromyawpitch(player1->yaw, player1->pitch, 1, 0, dir);
            player1->o.add(dir.mul(gotodist));
            player1->resetinterp();
        }
    }
    COMMANDN(goto, gotoplayer, "s");

    void gotoselection()
    {
        if(!m_edit)
        {
            conoutf(CON_ERROR, "\f9FanEd\f7::gotoselection: \f3#error: \f7\"gotosel\" only works in coop edit mode");
            playsound(S_ERROR);
            return;
        }
        player1->o = getselpos();
        vec dir;
        vecfromyawpitch(player1->yaw, player1->pitch, 1, 0, dir);
        player1->o.add(dir.mul(gotodist));
        player1->resetinterp();
    }
    COMMANDN(gotosel, gotoselection, "");
}
// End: Fanatic Edition

