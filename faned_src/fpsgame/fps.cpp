// Start: Fanatic Edition
#include "game.h"
#include "engine.h"

VAR(deathcamerastate, 0, 0, 1);
VARP(deathcamera, 0, 0, 1);
VARP(sehud, 0, 1, 1);
extern int fontfix;
// End: Fanatic Edition

namespace game
{
    bool clientoption(const char *arg) { return false; }
    bool intermission = false;
    int following = -1, followdir = 0;
    int lasthit = 0, lastspawnattempt = 0;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int respawnent = -1;
    int savedammo[NUMGUNS];

    fpsent *player1 = NULL;
    vector<fpsent *> players;

    void taunt()
    {
        if(player1->state!=CS_ALIVE || player1->physstate<PHYS_SLOPE) return;
        if(lastmillis-player1->lasttaunt<1000) return;
        player1->lasttaunt = lastmillis;
        addmsg(N_TAUNT, "rc", player1);
    }
    COMMAND(taunt, "");

    ICOMMAND(getfollow, "", (),
    {
        fpsent *f = followingplayer();
        intret(f ? f->clientnum : -1);
    });

    void follow(char *arg)
    {
        if(arg[0] ? player1->state==CS_SPECTATOR : following>=0)
        {
            following = arg[0] ? parseplayer(arg) : -1;
            if(following==player1->clientnum) following = -1;
            followdir = 0;
            conoutf("follow %s", following>=0 ? "ON" : "OFF"); // Fanatic Edition
        }
    }
    COMMAND(follow, "s");

    void nextfollow(int dir)
    {
        if(player1->state!=CS_SPECTATOR || clients.empty())
        {
            stopfollowing();
            return;
        }
        int cur = following >= 0 ? following : (dir < 0 ? clients.length() - 1 : 0);
        loopv(clients)
        {
            cur = (cur + dir + clients.length()) % clients.length();
            if(clients[cur] && clients[cur]->state!=CS_SPECTATOR)
            {
                if(following<0) conoutf("follow ON"); // Fanatic Edition
                following = cur;
                followdir = dir;
                return;
            }
        }
        stopfollowing();
    }
    ICOMMAND(nextfollow, "i", (int *dir), nextfollow(*dir < 0 ? -1 : 1));

    const char *getclientmap() { return clientmap; }

    void resetgamestate()
    {
        if(m_classicsp)
        {
            clearmovables();
            clearmonsters();
            entities::resettriggers();
        }
        clearprojectiles();
        clearbouncers();
    }

    fpsent *spawnstate(fpsent *d)
    {
        d->respawn();
        d->spawnstate(gamemode);
        return d;
    }

    void respawnself()
    {
        if(ispaused()) return;
        if(m_mp(gamemode))
        {
            int seq = (player1->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
            if(player1->respawned!=seq) { addmsg(N_TRYSPAWN, "rc", player1); player1->respawned = seq; }
        }
        else
        {
            spawnplayer(player1);
            showscores(false);
            lasthit = 0;
            if(cmode) cmode->respawned(player1);
        }
    }

    fpsent *pointatplayer()
    {
        loopv(players) if(players[i] != player1 && intersect(players[i], player1->o, worldpos)) return players[i];
        return NULL;
    }

    void stopfollowing()
    {
        if(following<0) return;
        following = -1;
        followdir = 0;
        conoutf("follow off");
    }

    fpsent *followingplayer()
    {
        if(player1->state!=CS_SPECTATOR || following<0) return NULL;
        fpsent *target = getclient(following);
        if(target && target->state!=CS_SPECTATOR) return target;
        return NULL;
    }

    fpsent *hudplayer()
    {
        if(thirdperson) return player1;
        fpsent *target = followingplayer();
        return target ? target : player1;
    }

    void setupcamera()
    {
        fpsent *target = followingplayer();
        if(target)
        {
            player1->yaw = target->yaw;
            player1->pitch = target->state==CS_DEAD ? 0 : target->pitch;
            player1->o = target->o;
            player1->resetinterp();
        }
    }

    bool detachcamera()
    {
        fpsent *d = hudplayer();
        return d->state==CS_DEAD;
    }

    bool collidecamera()
    {
        switch(player1->state)
        {
            case CS_EDITING: return false;
            case CS_SPECTATOR: return followingplayer()!=NULL;
        }
        return true;
    }

    VARP(smoothmove, 0, 75, 100);
    VARP(smoothdist, 0, 32, 64);

    void predictplayer(fpsent *d, bool move)
    {
        d->o = d->newpos;
        d->yaw = d->newyaw;
        d->pitch = d->newpitch;
        d->roll = d->newroll;
        if(move)
        {
            moveplayer(d, 1, false);
            d->newpos = d->o;
        }
        float k = 1.0f - float(lastmillis - d->smoothmillis)/smoothmove;
        if(k > 0)
        {
            d->o.add(vec(d->deltapos).mul(k));
            d->yaw += d->deltayaw*k;
            if(d->yaw<0) d->yaw += 360;
            else if(d->yaw>=360) d->yaw -= 360;
            d->pitch += d->deltapitch*k;
            d->roll += d->deltaroll*k;
        }
    }

    // Start: Fanatic Edition
    VAR(_autoswitchclient, -1, -1, 127);
    VAR(_autoswitchmode, 0, 0, 2);

    ICOMMAND(autoswitchclientteam,   "i", (int *cn), { _autoswitchclient = *cn; _autoswitchmode = 1; });
    ICOMMAND(autounswitchclientteam, "i", (int *cn), { _autoswitchclient = *cn; _autoswitchmode = 2; });

    VARP(deathpanicsound, 0, 1, 1);
    // VARP(quadmillislight, 0, 1, 1);

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            fpsent *d = players[i];

            if(players[i]->clientnum == _autoswitchclient)
            {
                if(_autoswitchmode == 1 && strcmp(players[i]->team, player1->team) != 0)
                {
                    switchteam(players[i]->team);
                }

            if(_autoswitchmode == 2 && strcmp(players[i]->team, player1->team) == 0)
                {
                    strcmp(player1->team, "good") == 0 ? switchteam("evil") : switchteam("good");
                }
            }

            if(d == hudplayer() && deathpanicsound)
            {
                if(d->health <= 25 && d->state == CS_ALIVE && !m_insta && player1->state != CS_SPECTATOR)
                {
                    d->hurtchan = playsound(S_HEARTBEAT, NULL, NULL, NULL, -1, 1000, d->hurtchan);
                }
                else d->stopheartbeat();
            }

            // if(quadmillislight && d->quadmillis) adddynlight(d->o, 30, vec(1.0f, 0, 0), 500, 100, DL_FLASH, 0, vec(0, 0, 0), d);
            // End: Fanatic Edition

            if(d == player1 || d->ai) continue;

            if(d->state==CS_DEAD && d->ragdoll) moveragdoll(d);
            else if(!intermission)
            {
                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                if(d->quadmillis) entities::checkquad(curtime, d);
            }

            const int lagtime = totalmillis-d->lastupdate;
            if(!lagtime || intermission) continue;
            else if(lagtime>1000 && d->state==CS_ALIVE)
            {
                d->state = CS_LAGGED;
                continue;
            }
            if(d->state==CS_ALIVE || d->state==CS_EDITING)
            {
                if(smoothmove && d->smoothmillis>0) predictplayer(d, true);
                else moveplayer(d, 1, false);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true);
        }
    }

    VARFP(slowmosp, 0, 0, 1, { if(m_sp && !slowmosp) server::forcegamespeed(100); }); 

    void checkslowmo()
    {
        static int lastslowmohealth = 0;
        server::forcegamespeed(intermission ? 100 : clamp(player1->health, 25, 200));
        if(player1->health<player1->maxhealth && lastmillis-max(maptime, lastslowmohealth)>player1->health*player1->health/2)
        {
            lastslowmohealth = lastmillis;
            player1->health++;
        }
    }

    void spawnplayer(fpsent *d)
    {
        if(cmode) cmode->pickspawn(d);
        else findplayerspawn(d, d==player1 && respawnent>=0 ? respawnent : -1);
        spawnstate(d);
        if(d==player1)
        {
            if(editmode) d->state = CS_EDITING;
            else if(d->state != CS_SPECTATOR) d->state = CS_ALIVE;
        }
        else d->state = CS_ALIVE;
    }

    VARP(playrespawnsound, 0, 1, 1); // Fanatic Edition

    VARP(spawnwait, 0, 0, 1000);

    void respawn()
    {
        if(player1->state==CS_DEAD)
        {
            player1->attacking = false;
            int wait = cmode ? cmode->respawnwait(player1) : 0;
            if(wait>0)
            {
                lastspawnattempt = lastmillis;
                //conoutf(CON_GAMEINFO, "\f2you must wait %d second%s before respawn!", wait, wait!=1 ? "s" : "");
                return;
            }
            if(lastmillis < player1->lastpain + spawnwait) return;
            if(m_dmsp) { changemap(clientmap, gamemode); return; }
            respawnself();
            if(m_classicsp)
            {
                conoutf(CON_GAMEINFO, "\f2You wasted another life! The monsters stole your armour and some ammo...");
                loopi(NUMGUNS) if(i!=GUN_PISTOL && (player1->ammo[i] = savedammo[i]) > 5) player1->ammo[i] = max(player1->ammo[i]/3, 5);
            }
            if(playrespawnsound) playsound(S_RESPAWN); // Fanatic Edition
        }
    }

    VARP(autorespawn, 0, 0, 1); // Fanatic Edition

    void updateworld()
    {
        if(autorespawn) respawn(); // Fanatic Edition

        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();
        if(player1->state != CS_DEAD && !intermission)
        {
            if(player1->quadmillis) entities::checkquad(curtime, player1);
        }

        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveragdolls();
        gets2c();
        updatemovables(curtime);
        updatemonsters(curtime);
        if(player1->state == CS_DEAD)
        {
            if(player1->ragdoll) moveragdoll(player1);
            else if(lastmillis-player1->lastpain<2000)
            {
                player1->move = player1->strafe = 0;
                moveplayer(player1, 10, true);
            }
        }
        else if(!intermission)
        {
            if(player1->ragdoll) cleanragdoll(player1);
            moveplayer(player1, 10, true);
            swayhudgun(curtime);
            entities::checkitems(player1);
            if(m_sp)
            {
                if(slowmosp) checkslowmo();
                if(m_classicsp) entities::checktriggers();
            }
            else if(cmode) cmode->checkitems(player1);
        }
        if(player1->clientnum >= 0) c2sinfo();
    }

    // Start: Fanatic Edition
    void doattack(bool on)
    {
        if(intermission) return;
        if((player1->attacking = on)) respawn();
        if(deathcamerastate)
        {
            deathcamerastate = 0;
            following = -1;
            followdir = 0;
            player1->state = CS_DEAD;
            respawn();
        }
    }

    void docrouch(int down)
    {
        if(down)
        {
            player1->eyeheight = player1->eyeheight - 5;
            player1->crouched = true;
            playsound(S_CROUCHIN);
        }
        else 
        {
            player1->eyeheight = player1->eyeheight + 5;
            player1->o.z = player1->o.z + 5;
            player1->crouched = false;
            playsound(S_CROUCHOUT);
        }
        player1->resetinterp();
    }
    ICOMMAND(crouch, "D", (int *down), docrouch(*down));

    bool canjump()
    {
        if(!intermission) respawn();
        if(deathcamerastate)
        {
            deathcamerastate = 0;
            following = -1;
            followdir = 0;
            player1->state = CS_DEAD;
            respawn();
        }
        return player1->state!=CS_DEAD && !intermission;
    }
    // End: Fanatic Edition

    bool allowmove(physent *d)
    {
        if(d->type!=ENT_PLAYER) return true;
        return !((fpsent *)d)->lasttaunt || lastmillis-((fpsent *)d)->lasttaunt>=1000;
    }

    // Start: Fanatic Edition
    VARP(hitsound, 0, 1, 1);
    VARP(damagemotion, 0, 1, 1);
    VAR(inmotion, 0, 0, 1);

    void dodamagemotion()
    {
        inmotion = 1;

        player1->roll = player1->roll+(rand()%2?+360:-360);

        execute("sleep 666 [motionblur $_motionblur]");
        execute("sleep 666 [motionblurmillis $_motionblurmillis]");
        execute("sleep 666 [motionblurscale $_motionblurscale]");
        execute("sleep 666 [inmotion 0]");
    }

    void damaged(int damage, fpsent *d, fpsent *actor, bool local)
    {
        if(d == player1 && damagemotion && !inmotion && (actor->gunselect == GUN_RL || actor->gunselect == GUN_GL))
        {
            execute("_motionblur = $motionblur");
            execute("_motionblurmillis = $motionblurmillis");
            execute("_motionblurscale = $motionblurscale");

            execute("motionblur 1");
            execute("motionblurmillis 100");
            execute("motionblurscale 0.5");

            dodamagemotion();
        }
        // End: Fanatic Edition

        if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        if(local) damage = d->dodamage(damage);
        else if(actor==player1) return;

        fpsent *h = hudplayer();
        if(h!=player1 && actor==h && d!=actor)
        {
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }
        if(d==h)
        {
            damageblend(damage);
            damagecompass(damage, actor->o);
        }
        damageeffect(damage, d, d!=h);

        ai::damaged(d, actor);

        if(m_sp && slowmosp && d==player1 && d->health < 1) d->health = 1;

        if(d->health<=0) { if(local) killed(d, actor); }
        else if(d==h) playsound(S_PAIN6);
        else playsound(S_PAIN1+rnd(5), &d->o);
    }

    VARP(deathscore, 0, 0, 1);

    void deathstate(fpsent *d, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;
        if(!restore) gibeffect(max(-d->health, 0), d->vel, d);
        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            if(!restore) loopi(NUMGUNS) savedammo[i] = player1->ammo[i];
            d->attacking = false;
            if(!restore) d->deaths++;
            //d->pitch = 0;
            d->roll = 0;
            playsound(S_DIE1+rnd(2));
        }
        else
        {
            if(!restore) d->deaths++; // Fanatic Edition
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playsound(S_DIE1+rnd(2), &d->o);
        }
    }

    VARP(teamcolorfrags, 0, 1, 1);

    // Start: Fanatic Edition
    VARP(autosaysorry, 0, 0, 1);
    SVARP(autosaysorrymsg, "Sorry, %s!");

    VARP(autosaynp, 0, 0, 1);
    SVARP(autosaynpmsg, "No problem, %s!");

    extern void sayteam(char *text);
    VARP(autosayteam, 0, 1, 1);

    VARP(guncolorfrags, 0, 1, 1);

    int centerconsolespecies1 = 0;
    int centerconsolespecies2 = 0;
    int centerconsoletime1 = 0;
    int centerconsoletime2 = 0;
    string centerconsolestring;
    string centerconsolemessage1;
    string centerconsolemessage2;
    int greatshot = 0;

    void killed(fpsent *d, fpsent *actor)
    {
        const char *you = "";
        if(!joinred)
        {
            if(m_teammode && teamcolorfrags) you = "\fs\f1you\fr";
            else you = "\fs\f2you\fr";
        }
        else
        {
            if(m_teammode && teamcolorfrags) you = "\fs\f3you\fr";
            else you = "\fs\f2you\fr";
        }

        const char *fragged = "";
        if(!guncolorfrags || m_insta) fragged = "\fs\f2fragged\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_FIST)   fragged = "\fs\f2dismembered\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_SG)     fragged = "\fs\f3peppered\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_CG)     fragged = "\fs\f0shredded\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_RIFLE)  fragged = "\fs\f1punctured\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_RL)     fragged = "\fs\f6nuked\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_GL)     fragged = "\fs\fIbusted\fr";
        if(guncolorfrags && !m_insta && actor->gunselect == GUN_PISTOL) fragged = "\fs\f4shot\fr";

        if(d->state == CS_EDITING)
        {
            d->editstate = CS_DEAD;
            d->deaths++;
            d->resetinterp();
            return;
        }
        else if((d->state != CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        fpsent *h = followingplayer();
        if(!h) h = player1;
        int contype = d == h || actor == h ? CON_FRAG_SELF : CON_FRAG_OTHER;
        const char *dname = "", *aname = "";
        if(m_teammode && teamcolorfrags)
        {
            dname = teamcolorname(d, "you");
            aname = teamcolorname(actor, "you");
        }
        else
        {
            dname = colorname(d, NULL, "", "", "you");
            aname = colorname(actor, NULL, "", "", "you");
        }
        if(actor->type == ENT_AI)
        {
            centerconsolespecies2 = 1;
            centerconsoletime2 = totalmillis;
            defformatstring(centerconsolestring)("\f2%s got %s by %s", you, fragged, aname);
            copystring(centerconsolemessage2, centerconsolestring);
            conoutf(contype, "%s", centerconsolestring);
        }
        else if(d == actor || actor->type == ENT_INANIMATE)
        {
            d->suicides++;
            if(d != player1) conoutf(contype, "\f2%s suicided", dname);
            else
            {
                centerconsolespecies2 = 1;
                centerconsoletime2 = totalmillis;
                defformatstring(centerconsolestring)("\f2you suicided");
                copystring(centerconsolemessage2, centerconsolestring);
                conoutf(contype, "\f2you suicided");
            }
            if(identexists("onsuicide"))
            {
                defformatstring(str)("onsuicide %d", d->clientnum);
                execute(str);
            }
        }
        else if(isteam(d->team, actor->team))
        {
            actor->teamkills++;
            contype |= CON_TEAMKILL;
            if(actor == player1)
            {
                centerconsolespecies1 = 1;
                centerconsoletime1 = totalmillis;
                defformatstring(centerconsolestring)("\f6%s %s your teammate %s", you, fragged, dname);
                copystring(centerconsolemessage1, centerconsolestring);
                conoutf(contype, "%s", centerconsolestring);

                if(autosaysorry)
                {
                    defformatstring(msg)(autosaysorrymsg, dname);
                    if(autosayteam) sayteam(msg);
                    else toserver(msg);
                }
            }
            else if(d == player1)
            {
                centerconsolespecies2 = 1;
                centerconsoletime2 = totalmillis;
                defformatstring(centerconsolestring)("\f6%s got %s by your teammate %s", you, fragged, aname);
                copystring(centerconsolemessage2, centerconsolestring);
                conoutf(contype, "%s", centerconsolestring);
                
                if(autosaynp)
                {
                    defformatstring(msg)(autosaynpmsg, aname);
                    if(autosayteam) sayteam(msg);
                    else toserver(msg);
                }
            }
            else
            {
                defformatstring(centerconsolestring)("\f2%s %s a teammate (%s)", aname, fragged, dname);
                conoutf(contype, "%s", centerconsolestring);
            }
        }
        else
        {
            if(d == player1)
            {
                centerconsolespecies2 = 1;
                centerconsoletime2 = totalmillis;
                defformatstring(centerconsolestring)("\f2%s got %s by %s", you, fragged, aname);
                copystring(centerconsolemessage2, centerconsolestring);
                conoutf(contype, "%s", centerconsolestring);
            }
            else
            {
                defformatstring(centerconsolestring)("\f2%s %s %s", aname, fragged, dname);
                conoutf(contype, "%s", centerconsolestring);

                if(actor == player1)
                {
                    centerconsolespecies1 = 1;
                    centerconsoletime1 = totalmillis;
                    defformatstring(centerconsolestring)("\f2%s %s %s", you, fragged, dname);
                    copystring(centerconsolemessage1, centerconsolestring);

                    if(d->timeinair)
                    {
                        playsound(S_GREATSHOT);
                        greatshot = 1;
                    }
                    else greatshot = 0;
                }
            }
        }

        deathstate(d);
        ai::killed(d, actor);
 
        if(d == player1 && actor != player1 && deathcamera)
        {
            deathcamerastate = 1;
            showscores(false);
            player1->state = CS_SPECTATOR;
            following = actor->clientnum;
            followdir = 0;
            d->stopheartbeat();
        }
    }
    // End: Fanatic Edition

    // Start: Fanatic Edition
    ICOMMAND(getaccuracy, "", (), intret((player1->totaldamage*100)/max(player1->totalshots, 1)));
    ICOMMAND(getarmour, "", (), intret(player1->armour));
    ICOMMAND(getcamposx, "", (), intret(player1->o.x));
    ICOMMAND(getcamposy, "", (), intret(player1->o.y));
    ICOMMAND(getcamposz, "", (), intret(player1->o.z));
    ICOMMAND(getdeaths, "", (), intret(player1->deaths));
    ICOMMAND(getflags, "", (), intret(player1->flags));
    ICOMMAND(getfrags, "", (), intret(player1->frags));
    ICOMMAND(getgunselect, "", (), intret(player1->gunselect));
    ICOMMAND(gethealth, "", (), intret(player1->health));
    ICOMMAND(getphysstate, "", (), intret(player1->physstate));
    ICOMMAND(getping, "", (), intret(player1->ping));
    ICOMMAND(getpitch, "", (), intret(player1->pitch));
    ICOMMAND(getspeed, "", (), intret(player1->vel.magnitude()));
    ICOMMAND(getstate, "", (), intret(player1->state));
    ICOMMAND(getsuicides, "", (), intret(player1->suicides));
    ICOMMAND(getteamkills, "", (), intret(player1->teamkills));
    ICOMMAND(gettotaldamage, "", (), intret(player1->totaldamage));
    ICOMMAND(gettotalshots, "", (), intret(player1->totalshots));
    ICOMMAND(getvelx, "", (), intret(player1->vel.x));
    ICOMMAND(getvely, "", (), intret(player1->vel.y));
    ICOMMAND(getvelz, "", (), intret(player1->vel.z));
    ICOMMAND(getyaw, "", (), intret(player1->yaw));

    int getteamscore(const char *team)
    {
        vector<teamscore> teamscores;
        if(cmode) cmode->getteamscores(teamscores);
        else loopv(players) if(players[i]->team[0])
        {
            fpsent *player = players[i];
            teamscore *ts = NULL;
            loopvj(teamscores) if(!strcmp(teamscores[j].team, player->team)) { ts = &teamscores[j]; break; }
            if(!ts) teamscores.add(teamscore(player->team, player->frags));
            else ts->score += player->frags;
        }
        loopv(teamscores)
        {
            if(!strcmp(teamscores[i].team, team))
            {
                return teamscores[i].score;
            }
        }
        return 0;
    }
    ICOMMAND(getteamscore, "s", (const char *team), intret(getteamscore(team)));
    // End: Fanatic Edition

    // Start: Fanatic Edition
    VARP(autosaygg, 0, 0, 1);
    SVARP(autosayggmsg, "gg");

    void timeupdate(int secs)
    {
        if(secs > 0)
        {
            maplimit = lastmillis + secs*1000;
        }
        else
        {
            intermission = true;
            player1->attacking = false;
            if(cmode) cmode->gameover();

            disablezoom();
            showscores(true);
            playsound(S_INTERMISSION);

            if(autosaygg) toserver(autosayggmsg);

            if(identexists("intermission")) execute("intermission"); // Legacy
            if(identexists("onintermission")) execute("onintermission");

            if(m_teammode && player1->state != CS_SPECTATOR)
            {
                int scoregood = getteamscore("good");
                int scoreevil = getteamscore("evil");
                if(strcmp(player1->team, "good") == 0 && scoregood > scoreevil) playsound(S_WIN);
                else if(strcmp(player1->team, "good") == 0 && scoregood < scoreevil) playsound(S_LOSE);
                else if(strcmp(player1->team, "evil") == 0 && scoregood < scoreevil) playsound(S_WIN);
                else if(strcmp(player1->team, "evil") == 0 && scoregood > scoreevil) playsound(S_LOSE);
                else if(scoregood == scoreevil) playsound(S_TIE);
            }

            conoutf(CON_GAMEINFO, "\f2intermission:");
            conoutf(CON_GAMEINFO, "\f2game has ended!");
            if(m_ctf) conoutf(CON_GAMEINFO, "\f2player frags: %d, flags: %d, deaths: %d, teamkills: %d, suicides: %d", player1->frags, player1->flags, player1->deaths, player1->teamkills, player1->suicides);
            else if(m_collect) conoutf(CON_GAMEINFO, "\f2player frags: %d, skulls: %d, deaths: %d, teamkills: %d, suicides: %d", player1->frags, player1->flags, player1->deaths, player1->teamkills, player1->suicides);
            else if(m_teammode) conoutf(CON_GAMEINFO, "\f2player frags: %d, deaths: %d, teamkills: %d, suicides: %d", player1->frags, player1->deaths, player1->teamkills, player1->suicides);
            else conoutf(CON_GAMEINFO, "\f2player frags: %d, deaths: %d", player1->frags, player1->deaths);
            int accuracy = (player1->totaldamage*100)/max(player1->totalshots, 1);
            conoutf(CON_GAMEINFO, "\f2player total damage dealt: %d, damage wasted: %d, accuracy(%%): %d", player1->totaldamage, player1->totalshots-player1->totaldamage, accuracy);
            if(m_sp) spsummary(accuracy);
        }
    }
    // End: Fanatic Edition
    
    vector<fpsent *> clients;

    fpsent *newclient(int cn)
    {
        if(cn < 0 || cn > max(0xFF, MAXCLIENTS + MAXBOTS))
        {
            neterr("clientnum", false);
            return NULL;
        }

        if(cn == player1->clientnum) return player1;

        while(cn >= clients.length()) clients.add(NULL);
        if(!clients[cn])
        {
            fpsent *d = new fpsent;
            d->clientnum = cn;
            clients[cn] = d;
            players.add(d);
        }
        return clients[cn];
    }

    fpsent *getclient(int cn)
    {
        if(cn == player1->clientnum) return player1;
        return clients.inrange(cn) ? clients[cn] : NULL;
    }

    void clientdisconnected(int cn, bool notify)
    {
        if(!clients.inrange(cn)) return;
        if(following==cn)
        {
            if(followdir) nextfollow(followdir);
            else stopfollowing();
        }
        unignore(cn);
        fpsent *d = clients[cn];
        if(!d) return;
        if(notify && d->name[0]) conoutf("\f4disconnected:\f7 %s", colorname(d));
        removeweapons(d);
        removetrackedparticles(d);
        removetrackeddynlights(d);
        if(cmode) cmode->removeplayer(d);
        players.removeobj(d);
        DELETEP(clients[cn]);
        cleardynentcache();
    }

    void clearclients(bool notify)
    {
        loopv(clients) if(clients[i]) clientdisconnected(i, notify);
    }

    void initclient()
    {
        player1 = spawnstate(new fpsent);
        filtertext(player1->name, "unnamed", false, MAXNAMELEN);
        players.add(player1);
    }

    VARP(showmodeinfo, 0, 1, 1);

    void startgame()
    {
        clearmovables();
        clearmonsters();
        clearprojectiles();
        clearbouncers();
        clearragdolls();
        clearteaminfo();

        loopv(players)
        {
            fpsent *d = players[i];
            d->frags = d->flags = 0;
            d->deaths = 0;
            d->totaldamage = 0;
            d->totalshots = 0;
            d->suicides = 0;
            d->teamkills = 0;
            d->maxhealth = 100;
            d->lifesequence = -1;
            d->respawned = d->suicided = -2;
        }

        setclientmode();

        intermission = false;
        maptime = maprealtime = 0;
        maplimit = -1;

        if(cmode)
        {
            cmode->preload();
            cmode->setup();
        }

        conoutf(CON_GAMEINFO, "\f7game mode is %s", server::modename(gamemode));

        if(m_sp)
        {
            defformatstring(scorename)("bestscore_%s", getclientmap());
            const char *best = getalias(scorename);
            if(*best) conoutf(CON_GAMEINFO, "\f2try to beat your best score so far: %s", best);
        }
        else
        {
            const char *info = m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
            if(showmodeinfo && info) conoutf(CON_GAMEINFO, "\f7%s", info);
        }

        if(player1->playermodel != playermodel) switchplayermodel(playermodel);

        showscores(false);
        disablezoom();
        lasthit = 0;

        // Start: Fanatic Edition
        if(identexists("mapstart")) execute("mapstart"); // Legacy
        if(identexists("onmapstart")) execute("onmapstart");
        player1->crouched = false;
        // End: Fanatic Edition
    }

    void startmap(const char *name)
    {
        // Start: Fanatic Edition
        deathcamerastate = 0;
        if(!m_edit) resetphysics();
        // End: Fanatic Edition
        ai::savewaypoints();
        ai::clearwaypoints(true);
        respawnent = -1;
        if(!m_mp(gamemode)) spawnplayer(player1);
        else findplayerspawn(player1, -1);
        entities::resetspawns();
        copystring(clientmap, name ? name : "");
        sendmapinfo();
    }

    const char *getmapinfo()
    {
        return showmodeinfo && m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        if(d->type==ENT_INANIMATE) return;
        if     (waterlevel>0) { if(material!=MAT_LAVA) playsound(S_SPLASH1, d==player1 ? NULL : &d->o); }
        else if(waterlevel<0) playsound(material==MAT_LAVA ? S_BURN : S_SPLASH2, d==player1 ? NULL : &d->o);
        if     (floorlevel>0) { if(d==player1 || d->type!=ENT_PLAYER || ((fpsent *)d)->ai) msgsound(S_JUMP, d); }
        else if(floorlevel<0) { if(d==player1 || d->type!=ENT_PLAYER || ((fpsent *)d)->ai) msgsound(S_LAND, d); }
    }

    void dynentcollide(physent *d, physent *o, const vec &dir)
    {
        switch(d->type)
        {
            case ENT_AI: if(dir.z > 0) stackmonster((monster *)d, o); break;
            case ENT_INANIMATE: if(dir.z > 0) stackmovable((movable *)d, o); break;
        }
    }

    void msgsound(int n, physent *d)
    {
        if(!d || d==player1)
        {
            addmsg(N_SOUND, "ci", d, n);
            playsound(n);
        }
        else
        {
            if(d->type==ENT_PLAYER && ((fpsent *)d)->ai)
            addmsg(N_SOUND, "ci", d, n);
            playsound(n, &d->o);
        }
    }

    // Start: Fanatic Edition
    void playasound(int snd)
    {
        msgsound(snd, player1);
    }

    void playasoundat(int snd, int cn)
    {
        int found = -1;
        loopv(players)
        {
            if(players[i]->clientnum == cn) found = i;
        }
        if(found > -1)
        {
            msgsound(snd, players[found]);
        }
    }

    ICOMMAND(playasound, "i", (int *s), { playasound(*s); } );
    ICOMMAND(playasoundat, "ii", (int *s, int *c), { playasoundat(*s, *c); } );
    // End: Fanatic Edition

    int numdynents() { return players.length()+monsters.length()+movables.length(); }

    dynent *iterdynents(int i)
    {
        if(i<players.length()) return players[i];
        i -= players.length();
        if(i<monsters.length()) return (dynent *)monsters[i];
        i -= monsters.length();
        if(i<movables.length()) return (dynent *)movables[i];
        return NULL;
    }

    bool duplicatename(fpsent *d, const char *name = NULL, const char *alt = NULL)
    {
        if(!name) name = d->name;
        if(alt && d != player1 && !strcmp(name, alt)) return true;
        loopv(players) if(d!=players[i] && !strcmp(name, players[i]->name)) return true;
        return false;
    }

    static string cname[3];
    static int cidx = 0;

    const char *colorname(fpsent *d, const char *name, const char *prefix, const char *suffix, const char *alt)
    {
        if(!name) name = alt && d == player1 ? alt : d->name; 
        bool dup = !name[0] || duplicatename(d, name, alt) || d->aitype != AI_NONE;
        if(dup || prefix[0] || suffix[0])
        {
            cidx = (cidx+1)%3;
            if(dup) formatstring(cname[cidx])(d->aitype == AI_NONE ? "%s%s \fs\f5(%d)\fr%s" : "%s%s \fs\f5[%d]\fr%s", prefix, name, d->clientnum, suffix); // Fanatic Edition
            else formatstring(cname[cidx])("%s%s%s", prefix, name, suffix);
            return cname[cidx];
        }
        return name;
    }

    VARP(teamcolortext, 0, 1, 1);

    const char *teamcolorname(fpsent *d, const char *alt)
    {
        if(!teamcolortext || !m_teammode) return colorname(d, NULL, "", "", alt);
        if(!joinred) return colorname(d, NULL, isteam(d->team, player1->team) ? "\fs\f1" : "\fs\f3", "\fr", alt); 
        else return colorname(d, NULL, isteam(d->team, player1->team) ? "\fs\f3" : "\fs\f1", "\fr", alt); 
    }

    const char *teamcolor(const char *name, bool sameteam, const char *alt)
    {
        if(!teamcolortext || !m_teammode) return sameteam || !alt ? name : alt;
        cidx = (cidx+1)%3;
        if(!joinred) formatstring(cname[cidx])(sameteam ? "\fs\f1%s\fr" : "\fs\f3%s\fr", sameteam || !alt ? name : alt);
        else formatstring(cname[cidx])(sameteam ? "\fs\f3%s\fr" : "\fs\f1%s\fr", sameteam || !alt ? name : alt);
        return cname[cidx];
    }    
    
    const char *teamcolor(const char *name, const char *team, const char *alt)
    {
        return teamcolor(name, team && isteam(team, player1->team), alt);
    }

    void suicide(physent *d)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((fpsent *)d)->ai))
        {
            if(d->state!=CS_ALIVE) return;
            fpsent *pl = (fpsent *)d;
            if(!m_mp(gamemode)) killed(pl, pl);
            else 
            {
                int seq = (pl->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
                if(pl->suicided!=seq) { addmsg(N_SUICIDE, "rc", pl); pl->suicided = seq; }
            }
        }
        else if(d->type==ENT_AI) suicidemonster((monster *)d);
        else if(d->type==ENT_INANIMATE) suicidemovable((movable *)d);
    }
    ICOMMAND(suicide, "", (), suicide(player1));

    bool needminimap() { return m_ctf || m_protect || m_hold || m_capture || m_collect; }

    void drawicon(int icon, float x, float y, float sz)
    {
        // Start: Fanatic Edition
        if(!joinred) settexture("faned/hud/items.png");
        else settexture("faned/hud/items_reversed.png");
        // End: Fanatic Edition
        glBegin(GL_TRIANGLE_STRIP);
        float tsz = 0.25f, tx = tsz*(icon%4), ty = tsz*(icon/4);
        glTexCoord2f(tx,     ty);  glVertex2f(x,    y);
        glTexCoord2f(tx+tsz, ty);  glVertex2f(x+sz, y);
        glTexCoord2f(tx,     ty+tsz); glVertex2f(x,    y+sz);
        glTexCoord2f(tx+tsz, ty+tsz); glVertex2f(x+sz, y+sz);
        glEnd();
    }

    float abovegameplayhud(int w, int h)
    {
        switch(hudplayer()->state)
        {
            case CS_EDITING:
            case CS_SPECTATOR:
                return 1;
            default:
                return 1650.0f/1800.0f;
        }
    }

    int ammohudup[3] = { GUN_CG, GUN_RL, GUN_GL },
        ammohuddown[3] = { GUN_RIFLE, GUN_SG, GUN_PISTOL },
        ammohudcycle[7] = { -1, -1, -1, -1, -1, -1, -1 };

    ICOMMAND(ammohudup, "V", (tagval *args, int numargs),
    {
        loopi(3) ammohudup[i] = i < numargs ? getweapon(args[i].getstr()) : -1;
    });

    ICOMMAND(ammohuddown, "V", (tagval *args, int numargs),
    {
        loopi(3) ammohuddown[i] = i < numargs ? getweapon(args[i].getstr()) : -1;
    });

    ICOMMAND(ammohudcycle, "V", (tagval *args, int numargs),
    {
        loopi(7) ammohudcycle[i] = i < numargs ? getweapon(args[i].getstr()) : -1;
    });

    VARP(ammohud, 0, 1, 1);

    void drawammohud(fpsent *d)
    {
        float x = HICON_X + 2*HICON_STEP, y = HICON_Y, sz = HICON_SIZE;
        glPushMatrix();
        glScalef(1/3.2f, 1/3.2f, 1);
        float xup = (x+sz)*3.2f, yup = y*3.2f + 0.1f*sz;
        loopi(3)
        {
            int gun = ammohudup[i];
            if(gun < GUN_FIST || gun > GUN_PISTOL || gun == d->gunselect || !d->ammo[gun]) continue;
            drawicon(HICON_FIST+gun, xup, yup, sz);
            yup += sz;
        }
        float xdown = x*3.2f - sz, ydown = (y+sz)*3.2f - 0.1f*sz;
        loopi(3)
        {
            int gun = ammohuddown[3-i-1];
            if(gun < GUN_FIST || gun > GUN_PISTOL || gun == d->gunselect || !d->ammo[gun]) continue;
            ydown -= sz;
            drawicon(HICON_FIST+gun, xdown, ydown, sz);
        }
        int offset = 0, num = 0;
        loopi(7)
        {
            int gun = ammohudcycle[i];
            if(gun < GUN_FIST || gun > GUN_PISTOL) continue;
            if(gun == d->gunselect) offset = i + 1;
            else if(d->ammo[gun]) num++;
        }
        float xcycle = (x+sz/2)*3.2f + 0.5f*num*sz, ycycle = y*3.2f-sz;
        loopi(7)
        {
            int gun = ammohudcycle[(i + offset)%7];
            if(gun < GUN_FIST || gun > GUN_PISTOL || gun == d->gunselect || !d->ammo[gun]) continue;
            xcycle -= sz;
            drawicon(HICON_FIST+gun, xcycle, ycycle, sz);
        }
        glPopMatrix();
    }

    void drawhudicons(fpsent *d)
    {
        if(sehud) return; // Fanatic Edition
        glPushMatrix();
        glScalef(2, 2, 1);

        draw_textf("%d", (HICON_X + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->state==CS_DEAD ? 0 : d->health);
        if(d->state!=CS_DEAD)
        {
            if(d->armour) draw_textf("%d", (HICON_X + HICON_STEP + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->armour);
            draw_textf("%d", (HICON_X + 2*HICON_STEP + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->ammo[d->gunselect]);
        }

        glPopMatrix();

        drawicon(HICON_HEALTH, HICON_X, HICON_Y);
        if(d->state!=CS_DEAD)
        {
            if(d->armour) drawicon(HICON_BLUE_ARMOUR+d->armourtype, HICON_X + HICON_STEP, HICON_Y);
            drawicon(HICON_FIST+d->gunselect, HICON_X + 2*HICON_STEP, HICON_Y);
            if(d->quadmillis) drawicon(HICON_QUAD, HICON_X + 3*HICON_STEP, HICON_Y);
            if(ammohud) drawammohud(d);
        }
    }

    // Start: Fanatic Edition
    void drawsehud(int w, int h, fpsent *d)
    {
        glPushMatrix();
        glScalef(1/1.2f, 1/1.2f, 1);
        if(!m_insta) draw_textf("%d", 80, h*1.2f-(fontfix ? 141 : 136), max(0, d->health));
        defformatstring(ammo)("%d", player1->ammo[d->gunselect]);
        int wb, hb;
        text_bounds(ammo, wb, hb);
        draw_textf("%d", w*1.2f-wb-80, h*1.2f-(fontfix ? 141 : 136), d->ammo[d->gunselect]);

        if(d->quadmillis)
        {
            settexture("faned/hud/se_hud_quaddamage_left.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);   glVertex2f(0,   h*1.2f-207);
            glTexCoord2f(1.0f, 0.0f);   glVertex2f(539, h*1.2f-207);
            glTexCoord2f(1.0f, 1.0f);   glVertex2f(539, h*1.2f);
            glTexCoord2f(0.0f, 1.0f);   glVertex2f(0,   h*1.2f);
            glEnd();

            settexture("faned/hud/se_hud_quaddamage_right.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);   glVertex2f(w*1.2f-135, h*1.2f-207);
            glTexCoord2f(1.0f, 0.0f);   glVertex2f(w*1.2f,     h*1.2f-207);
            glTexCoord2f(1.0f, 1.0f);   glVertex2f(w*1.2f,     h*1.2f);
            glTexCoord2f(0.0f, 1.0f);   glVertex2f(w*1.2f-135, h*1.2f);
            glEnd();
        }

        if(d->maxhealth > 100)
        {
            settexture("faned/hud/se_hud_megahealth.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);   glVertex2f(0,   h*1.2f-207);
            glTexCoord2f(1.0f, 0.0f);   glVertex2f(539, h*1.2f-207);
            glTexCoord2f(1.0f, 1.0f);   glVertex2f(539, h*1.2f);
            glTexCoord2f(0.0f, 1.0f);   glVertex2f(0,   h*1.2f);
            glEnd();
        }

        int health = (d->health*100)/d->maxhealth,
        armour = (d->armour*100)/200,
        hh = (health*101)/100,
        ah = (armour*167)/100;

        float hs = (health*1.0f)/100, as = (armour*1.0f)/100;

        if(d->health > 0 && !m_insta)
        {
            settexture("faned/hud/se_hud_health.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f-hs);   glVertex2f(47, h*1.2f-hh-56);
            glTexCoord2f(1.0f, 1.0f-hs);   glVertex2f(97, h*1.2f-hh-56);
            glTexCoord2f(1.0f, 1.0f);   glVertex2f(97, h*1.2f-57);
            glTexCoord2f(0.0f, 1.0f);   glVertex2f(47, h*1.2f-57);
            glEnd();
        }

        if(d->armour > 0)
        {
            settexture("faned/hud/se_hud_armour.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,    0.0f);   glVertex2f(130,    h*1.2f-62);
            glTexCoord2f(as,      0.0f);   glVertex2f(130+ah, h*1.2f-62);
            glTexCoord2f(as,      1.0f);   glVertex2f(130+ah, h*1.2f-44);
            glTexCoord2f(0.0f,    1.0f);   glVertex2f(130,    h*1.2f-44);
            glEnd();
        }

        if(!m_insta)
        {
            settexture("faned/hud/se_hud_left.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);   glVertex2f(0,   h*1.2f-207);
            glTexCoord2f(1.0f, 0.0f);   glVertex2f(539, h*1.2f-207);
            glTexCoord2f(1.0f, 1.0f);   glVertex2f(539, h*1.2f);
            glTexCoord2f(0.0f, 1.0f);   glVertex2f(0,   h*1.2f);
            glEnd();
        }

        settexture("faned/hud/se_hud_right.png");
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);   glVertex2f(w*1.2f-135, h*1.2f-207);
        glTexCoord2f(1.0f, 0.0f);   glVertex2f(w*1.2f,     h*1.2f-207);
        glTexCoord2f(1.0f, 1.0f);   glVertex2f(w*1.2f,     h*1.2f);
        glTexCoord2f(0.0f, 1.0f);   glVertex2f(w*1.2f-135, h*1.2f);
        glEnd();

        int maxammo = 1;

        switch(d->gunselect)
        {
            case GUN_FIST:
                maxammo = 1;
                break;

            case GUN_RL:
            case GUN_RIFLE:
                maxammo = m_insta ? 100 : 15;
                break;

            case GUN_SG:
            case GUN_GL:
                maxammo = 30;
                break;

            case GUN_CG:
                maxammo = 60;
                break;

            case GUN_PISTOL:
                maxammo = 120;
                break;
        }

        int curammo = min((d->ammo[d->gunselect]*100)/maxammo, maxammo),
        amh = (curammo*101)/100;

        float ams = (curammo*1.0f)/100;

        if(d->ammo[d->gunselect] > 0)
        {
            settexture("faned/hud/se_hud_health.png");
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f-ams);   glVertex2f(w*1.2f-47, h*1.2f-amh-56);
            glTexCoord2f(1.0f, 1.0f-ams);   glVertex2f(w*1.2f-97, h*1.2f-amh-56);
            glTexCoord2f(1.0f, 1.0f);    glVertex2f(w*1.2f-97, h*1.2f-57);
            glTexCoord2f(0.0f, 1.0f);    glVertex2f(w*1.2f-47, h*1.2f-57);
            glEnd();
        }

        glPopMatrix();
        glPushMatrix();
        glScalef(1/4.0f, 1/4.0f, 1);
        defformatstring(icon)("faned/hud/se_hud_gun_%d.png", d->gunselect);
        settexture(icon);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);   glVertex2f(w*4.0f-1162,    h*4.0f-350);
        glTexCoord2f(1.0f, 0.0f);   glVertex2f(w*4.0f-650,     h*4.0f-350);
        glTexCoord2f(1.0f, 1.0f);   glVertex2f(w*4.0f-650,     h*4.0f-50);
        glTexCoord2f(0.0f, 1.0f);   glVertex2f(w*4.0f-1162,    h*4.0f-50);
        glEnd();
        glPopMatrix();
    }

    // Start: Fanatic Edition
    static inline bool playerslist(const fpsent *a, const fpsent *b)
    {
        if(a == player1) return true;
        if(b == player1) return false;
        if(a->state == CS_SPECTATOR)
        {
            if(b->state == CS_SPECTATOR) return strcmp(a->name, b->name) < 0;
            else return false;
        }
        else if(b->state == CS_SPECTATOR) return true;

        if(a->frags > b->frags) return true;
        if(a->frags < b->frags) return false;
        
        if(a->clientnum > b->clientnum) return true;
        if(a->clientnum < b->clientnum) return false;
        return false;
    }

    VARP(hudminiscoreboard, 0, 0, 1);
    VARP(hudminiscoreboardalpha, 0, 200, 255);
    SVARP(hudminiscoreboardcolor, "7");
    VARP(hudminiscoreboardlimit, 0, 30, 127);
    VARP(hudminiscoreboardlimitcmode, 0, 18, 127);

    VARP(hudstats, 0, 0, 1);
    VARP(hudstatsalpha, 0, 160, 255);
    SVARP(hudstatscolor, "7");

    void gameplayhud(int w, int h)
    {
        fpsent *d = hudplayer();

        if(!deathcamerastate && sehud && player1->state != CS_EDITING && player1->state != CS_SPECTATOR) drawsehud(w, h, d);

        glPushMatrix();
        glScalef(h/1800.0f, h/1800.0f, 1);

        int pw, ph, tw, th, fw, fh;
        text_bounds("  ", pw, ph);

        if(player1->state == CS_SPECTATOR)
        {
            if(!deathcamerastate)
            {
                text_bounds("SPECTATOR", tw, th);
                th = max(th, ph);
                fpsent *f = followingplayer();
                defformatstring(fplayer)("%s \f%d(%d)", f ? (duplicatename(f, f->name) ? f->name : colorname(f)) : NULL, f ? (duplicatename(f, f->name) ? 5 : 4) : NULL, f ? f->clientnum : NULL);
                text_bounds(f ? fplayer : " ", fw, fh);
                fh = max(fh, ph);
                draw_text("SPECTATOR", w*1800/h - tw - pw, 1650 - th - fh);
                if(f) 
                {
                    int color = f->state!=CS_DEAD ? 0xFFFFFF : 0x606060;
                    if(f->privilege)
                    {
                        color = f->privilege>=PRIV_ADMIN ? 0xFF8000 : 0x40FF80;
                        if(f->state == CS_DEAD) color = (color>>1)&0x7F7F7F;
                    }
                    draw_text(fplayer, w*1800/h - fw - pw, 1650 - fh, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
                }
            }

            else
            {
                text_bounds("DEATH CAMERA", tw, th);
                th = max(th, ph);
                fpsent *f = followingplayer();
                text_bounds(f ? colorname(f) : " ", fw, fh);
                fh = max(fh, ph);
                draw_text("\fZDEATH CAMERA", w*1800/h - tw - pw, 1650 - th - fh);
                if(f) 
                {
                    int color = f->state!=CS_DEAD ? 0xFFFFFF : 0x606060;
                    if(f->privilege)
                    {
                        color = f->privilege>=PRIV_ADMIN ? 0xFF8000 : 0x40FF80;
                        if(f->state == CS_DEAD) color = (color>>1)&0x7F7F7F;
                    }
                    draw_text(colorname(f), w*1800/h - fw - pw, 1650 - fh, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
                }
            }
        }

        if(!deathcamerastate && d->state!=CS_EDITING)
        {
            if(d->state!=CS_SPECTATOR) drawhudicons(d);
            if(cmode) cmode->drawhud(d, w, h);
        }

        if(!deathcamerastate && hudstats && player1->state != CS_EDITING && !getvar("scoreboard"))
        {
            static int lastfps = 0, prevfps[3] = { 0, 0, 0 }, curfps[3] = { 0, 0, 0 };
            if(totalmillis - lastfps >= 200)
            {
                memcpy(prevfps, curfps, sizeof(prevfps));
                lastfps = totalmillis - (totalmillis%200);
            }
            int nextfps[3];
            getfps(nextfps[0], nextfps[1], nextfps[2]);
            loopi(3) if(prevfps[i] == curfps[i]) curfps[i] = nextfps[i];

            fpsent *d = hudplayer();

            float speed = d->vel.magnitude();
            speed+= 0.5;
            int curspeed = d->state != CS_DEAD ? (int)speed : NULL;

            float kpd =  float(d->frags) / max(float(d->deaths), 1.0f);
            int accuracy = (d->totaldamage*100)/max(d->totalshots, 1);

            if(cmode)
            {
                draw_textfa("\f%sFPS:\nPing:\nSpeed:\n\n%s:\nFrags:\nK.p.D.:\nDeaths:\nAccuracy:", hudstatsalpha, 30, 420, hudstatscolor, m_collect ? "Skulls" : "Flags");
                draw_textfa("\f%s%d\n%d\n%d\n\n%d\n%d\n%4.2f\n%d\n%d%%", hudstatsalpha, 300, 420, hudstatscolor, curfps[0], d->ping, curspeed, d->flags, d->frags, kpd, d->deaths, accuracy);
            }
            else
            {
                draw_textfa("\f%sFPS:\nPing:\nSpeed:\n\nFrags:\nK.p.D.:\nDeaths:\nAccuracy:", hudstatsalpha, 30, 480, hudstatscolor);
                draw_textfa("\f%s%d\n%d\n%d\n\n%d\n%4.2f\n%d\n%d%%", hudstatsalpha, 300, 480, hudstatscolor, curfps[0], d->ping, curspeed, d->frags, kpd, d->deaths, accuracy);
            }
        }

        if(!deathcamerastate && hudminiscoreboard && player1->state != CS_EDITING && !getvar("scoreboard"))
        {
            glPushMatrix();
            glScalef(0.7, 0.7, 1);

            int iplayerssorted = 0;
            loopv(players)
            {
                players.sort(playerslist);

                int pw, ph, th, fw, fh;
                text_bounds("  ", pw, ph);
                th = max(th, ph);
                fpsent *f = players[i];
                defformatstring(fplayer)("%s \f%d(%d)", f ? (duplicatename(f, f->name) ? f->name : colorname(f)) : NULL, f ? (duplicatename(f, f->name) ? 5 : 4) : NULL, f ? f->clientnum : NULL);
                text_bounds(f ? fplayer : " ", fw, fh);
                fh = max(fh, ph);

                iplayerssorted++;
                if(iplayerssorted <= (cmode ? hudminiscoreboardlimitcmode : hudminiscoreboardlimit))
                {
                    fpsent *hudminiscoreboardplayer = players[i];
                    float kpd =  float(hudminiscoreboardplayer->frags) / max(float(hudminiscoreboardplayer->deaths), 1.0f);
                    int accuracy = (hudminiscoreboardplayer->totaldamage*100)/max(hudminiscoreboardplayer->totalshots, 1);

                    draw_textfa("\f%s%s \f4(%d)", hudminiscoreboardalpha, 1365*3 - fw - pw, cmode ? (443+i*30)*2 : (m_edit ? (29+i*30)*2 : (90+i*30)*2), hudminiscoreboardcolor, hudminiscoreboardplayer->name, hudminiscoreboardplayer->clientnum);
                    draw_textfa("\f%s%s%d %s%d %s%4.2f %d%% ", hudminiscoreboardalpha, 1200*3+460, cmode ? (443+i*30)*2 : (m_edit ? (29+i*30)*2 : (90+i*30)*2), hudminiscoreboardcolor, (hudminiscoreboardplayer->frags >= 0 && hudminiscoreboardplayer->frags < 10) ? "0" : "", hudminiscoreboardplayer->frags, hudminiscoreboardplayer->deaths <= 9 ? "0" : "", hudminiscoreboardplayer->deaths, (kpd >= 0 && kpd < 10) ? "0" : "", kpd, accuracy);
                }
            }
            glPopMatrix();
        }
        glPopMatrix();
    }
    // End: Fanatic Edition

    int clipconsole(int w, int h)
    {
        if(cmode) return cmode->clipconsole(w, h);
        return 0;
    }

    // Start: Fanatic Edition
    VARP(centerconsole, 0, 1, 1);
    VARP(centerconsolealpha, 0, 255, 255);
    VARP(centerconsoledelay, 0, 4000, 10000);

    VARP(hitcrosshair, 0, 425, 1000);

    const char *lastcrosshairinfoname;
    int lastcrosshairinfo;
    VARP(crosshairnames, 0, 1, 1);
    VARP(crosshairnamesalpha, 0, 80, 255);
    VARP(crosshairnamesdelay, 0, 250, 10000);
    SVARP(crosshairnamescolor, "7");

    VARP(teamcrosshair, 0, 0, 1);

    const char *defaultcrosshair(int index)
    {
        switch(index)
        {
            case 2: return "data/hit.png";
            case 1: return "data/teammate.png";
            default: return "data/crosshair.png";
        }
    }

    int selectcrosshair(float &r, float &g, float &b, int &w, int &h)
    {
        fpsent *d = hudplayer();

        if(centerconsole)
        {
            if(centerconsolespecies1 && totalmillis - centerconsoletime1 <= centerconsoledelay && greatshot)
            {
                int tw, th;
                text_bounds("Great Shot!", tw, th);
                glPushMatrix();
                glScalef(1/2.5f, 1/2.5f, 1);
                draw_textfa("\fTGreat Shot!", centerconsolealpha, (w*2.5f - tw)/2, h*2.5f-600);
                glPopMatrix();
            }

            if(centerconsolespecies1 && totalmillis - centerconsoletime1 <= centerconsoledelay)
            {
                int tw, th;
                text_bounds(centerconsolemessage1, tw, th);
                glPushMatrix();
                glScalef(1/2.5f, 1/2.5f, 1);
                draw_textfa("%s", centerconsolealpha, (w*2.5f - tw)/2, h*2.5f-500, centerconsolemessage1);
                glPopMatrix();
            }
    
            if(centerconsolespecies2 && totalmillis - centerconsoletime2 <= centerconsoledelay)
            {
                int tw, th;
                text_bounds(centerconsolemessage2, tw, th);
                glPushMatrix();
                glScalef(1/2.5f, 1/2.5f, 1);
                draw_textfa("%s", centerconsolealpha, (w*2.5f - tw)/2, h*2.5f-400, centerconsolemessage2);
                glPopMatrix();
            }
        }

        if(d->state == CS_SPECTATOR || d->state == CS_DEAD) return -1;

        if(d->state != CS_ALIVE) return 0;

        int crosshair = 0;

        if(lasthit && lastmillis - lasthit < hitcrosshair) crosshair = 2;

        else if(teamcrosshair)
        {
            dynent *o = intersectclosest(d->o, worldpos, d);
            if(o && o->type==ENT_PLAYER && isteam(((fpsent *)o)->team, d->team)) crosshair = 1;
        }

        if(crosshair!=1 && !editmode && !m_insta)
        {
            if(d->health<=25) { r = 1.0f; g = b = 0; }
            else if(d->health<=50) { r = 1.0f; g = 0.5f; b = 0; }
        }

        if(crosshairnames)
        {
            dynent *o = intersectclosest(d->o, worldpos, d);
            if(o && o->type==ENT_PLAYER)
            {
                lastcrosshairinfo = totalmillis;
                lastcrosshairinfoname = ((fpsent *)o)->name;
            }

            if(totalmillis - lastcrosshairinfo <= crosshairnamesdelay)
            {
                draw_textfa("\f%s%s", crosshairnamesalpha, w/2, h/3, crosshairnamescolor, lastcrosshairinfoname);
            }
        }

        if(d->gunwait > 500) crosshairbump();

        return crosshair;
    }
    // End: Fanatic Edition

    void lighteffects(dynent *e, vec &color, vec &dir)
    {
        #if 0
            fpsent *d = (fpsent *)e;
            if(d->state!=CS_DEAD && d->quadmillis)
            {
                float t = 0.5f + 0.5f*sinf(2*M_PI*lastmillis/1000.0f);
                color.y = color.y*(1-t) + t;
            }
        #endif
    }

    bool serverinfostartcolumn(g3d_gui *g, int i)
    {
        static const char * const names[] = { "ping ", "players ", "mode ", "map ", "time ", "master ", "host ", "port ", "description " };
        static const float struts[] =       { 7,       7,          12.5f,   14,      7,      8,         14,      7,       24.5f };
        if(size_t(i) >= sizeof(names)/sizeof(names[0])) return false;
        g->pushlist();
        g->text(names[i], 0x50CFE5, !i ? " " : NULL); // Fanatic Edition
        if(struts[i]) g->strut(struts[i]);
        g->mergehits(true);
        return true;
    }

    void serverinfoendcolumn(g3d_gui *g, int i)
    {
        g->mergehits(false);
        g->column(i);
        g->poplist();
    }

    const char *mastermodecolor(int n, const char *unknown)
    {
        return "\f7"; // (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodecolors)/sizeof(mastermodecolors[0])) ? mastermodecolors[n-MM_START] : unknown; // Fanatic Edition
    }

    const char *mastermodeicon(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodeicons)/sizeof(mastermodeicons[0])) ? mastermodeicons[n-MM_START] : unknown;
    }

    bool serverinfoentry(g3d_gui *g, int i, const char *name, int port, const char *sdesc, const char *map, int ping, const vector<int> &attr, int np)
    {
        if(ping < 0 || attr.empty() || attr[0]!=PROTOCOL_VERSION)
        {
        switch(i)
        {
            case 0:
                if(g->button(" ", 0xFFFFFF, "serverunk")&G3D_UP) return true;
                break;

            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                if(g->button(" ", 0xFFFFFF)&G3D_UP) return true;
                break;

            case 6:
                if(g->buttonf("%s ", 0xFFFFFF, NULL, name)&G3D_UP) return true;
                break;

            case 7:
                if(g->buttonf("%d ", 0xFFFFFF, NULL, port)&G3D_UP) return true;
                break;

            case 8:
                if(ping < 0)
                {
                    if(g->button(sdesc, 0xFFFFFF)&G3D_UP) return true;
                }
                else if(g->buttonf("[%s protocol] ", 0xFFFFFF, NULL, attr.empty() ? "unknown" : (attr[0] < PROTOCOL_VERSION ? "older" : "newer"))&G3D_UP) return true;
                break;
        }
        return false;
        }

        switch(i)
        {
        case 0:
        {
            const char *icon = attr.inrange(3) && np >= attr[3] ? "serverfull" : (attr.inrange(4) ? mastermodeicon(attr[4], "serverunk") : "serverunk");
            if(g->buttonf("%d ", 0xFFFFFF, icon, ping)&G3D_UP) return true;
            break;
        }

        case 1:
            if(attr.length()>=4)
            {
                if(g->buttonf("%d/%d ", 0xFFFFFF, NULL, np, attr[3])&G3D_UP) return true;
            }
            else if(g->buttonf("%d ", 0xFFFFFF, NULL, np)&G3D_UP) return true;
            break;

        case 2:
            if(g->buttonf("%s ", 0xFFFFFF, NULL, attr.length()>=2 ? server::modename(attr[1], "") : "")&G3D_UP) return true;
            break;

        case 3:
            if(g->buttonf("%.25s ", 0xFFFFFF, NULL, map)&G3D_UP) return true;
            break;

        case 4:
            if(attr.length()>=3 && attr[2] > 0)
            {
                int secs = clamp(attr[2], 0, 59*60+59),
                    mins = secs/60;
                secs %= 60;
                if(g->buttonf("%d:%02d ", 0xFFFFFF, NULL, mins, secs)&G3D_UP) return true;
            }
            else if(g->buttonf(" ", 0xFFFFFF)&G3D_UP) return true;
            break;
        case 5:
            if(g->buttonf("%s%s ", 0xFFFFFF, NULL, attr.length()>=5 ? mastermodecolor(attr[4], "") : "", attr.length()>=5 ? server::mastermodename(attr[4], "") : "")&G3D_UP) return true;
            break;

        case 6:
            if(g->buttonf("%s ", 0xFFFFFF, NULL, name)&G3D_UP) return true;
            break;

        case 7:
            if(g->buttonf("%d ", 0xFFFFFF, NULL, port)&G3D_UP) return true;
            break;

        case 8:
            if(g->buttonf("%.25s", 0xFFFFFF, NULL, sdesc)&G3D_UP) return true;
            break;
        }
        return false;
    }

    void writegamedata(vector<char> &extras) {}
    void readgamedata(vector<char> &extras) {}

    // Start: Fanatic Edition
    const char *faned_autoexec()       { return "faned_autoexec.cfg"; }
    const char *faned_config()         { return "faned_config.cfg"; }
    const char *faned_defaultconfig()  { return "faned/data/defaults.cfg"; }
    const char *faned_restoreconfig()  { return "faned_restore.cfg"; }
    const char *faned_scripts()        { return "faned/init.cfg"; }
    const char *faned_savedservers()   { return "faned_servers.cfg"; }
    const char *faned_sounds()         { return "faned/data/sounds.cfg"; }
    const char *faned_whois()          { return "faned_whois.db"; }

    void loadconfigs()
    {
        execfile("faned_auth.cfg", false);
    }
    // End: Fanatic Edition
}

