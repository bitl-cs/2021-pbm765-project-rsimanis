#include "pong_game.h"


/* init */
void init_team(team *team, char id, int active_player_count, float goal_x1, float goal_y1, float goal_x2, float goal_y2) {
    team->id = id;
    team->active_player_count = active_player_count;
    team->score = TEAM_INITIAL_SCORE;
    team->goal1.x = goal_x1;
    team->goal1.y = goal_y1;
    team->goal2.x = goal_x2;
    team->goal2.y = goal_y2;
}

void init_player(player *player, char id, char client_id, char team_id, char *name, float x, float y) {
    player->id = id;
    player->client_id = client_id;
    player->team_id = team_id;
    player->ready = PLAYER_READY_FALSE;
    player->name = name;
    player->score = PLAYER_INITIAL_SCORE;
    player->pos.x = x;
    player->pos.y = y;
    player->v.x = PLAYER_INITIAL_VELOCITY_X;
    player->v.y = PLAYER_INITIAL_VELOCITY_Y;
    player->a.x = PLAYER_INITIAL_ACCELERATION_X;
    player->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    player->width = PLAYER_INITIAL_WIDTH;
    player->height = PLAYER_INITIAL_HEIGHT;
}

void init_ball(ball *ball) {
    ball->pos.x = BALL_INITIAL_X;
    ball->pos.y = BALL_INITIAL_Y;
    init_ball_velocity(ball);
    ball->a.x = BALL_INITIAL_ACCELERATION_X;
    ball->a.y = BALL_INITIAL_ACCELERATION_Y;
    ball->type = BALL_TYPE_NORMAL;
    ball->radius = BALL_INITIAL_RADIUS;
    ball->last_touched_id = BALL_INITIAL_LAST_TOUCHED_ID;
}

void init_ball_velocity(ball *ball) {
    rand_vec2f(&ball->v, BALL_INITIAL_VELOCITY_MOD);
    while (angle_with_horizon_vec2f(&ball->v) > BALL_INIT_ANGLE_IN_DEGREES)
        rand_vec2f(&ball->v, BALL_INITIAL_VELOCITY_MOD);
}

void init_power_up(power_up *power_up, float x, float y, float width, float height, char type) {
    power_up->pos.x = x;
    power_up->pos.y = y;
    power_up->width = width;
    power_up->height = height;
    power_up->type = type;
}

void init_screen(game_state *gs) {
    gs->window_width = GAME_WINDOW_WIDTH;
    gs->window_height = GAME_WINDOW_HEIGHT;
}

void start_game(game_state *gs) {
    gs->status = GAME_STATE_STATUS_IN_PROGRESS;
}

/* gameloop */
void update_game_state(game_state *gs) {
    char i;

    /* check if some team has not disconnected */
    for (i = 0; i < gs->team_count; i++) {
        if (gs->teams[i].active_player_count == 0) {
            end_game(gs, GAME_STATE_STATUS_ALL_TEAM_LEFT);
            break;
        }
    }

    /* update players */
    for (i = 0; i < gs->player_count; i++) {
        if (gs->players[i].client_id != PLAYER_DISCONNECTED_CLIENT_ID)
            update_player(&gs->players[i], gs);
    }

    /* update balls */
    for (i = 0; i < gs->ball_count; i++)
        update_ball(&gs->balls[i], gs);

    /* update power-ups */
    for (i = 0; i < MAX_POWER_UP_COUNT; i++)
        update_power_up(&gs->power_ups[i], gs);
}

void update_power_up(power_up *pu, game_state *gs) {
    if (pu->type == -1 && time_diff_in_seconds(pu->last_spawn_time, clock()) >= POWER_UP_SPAWN_TIME) {
        spawn_power_up(pu);
        gs->power_up_count++;
    }
}

void spawn_power_up(power_up *pu) {
    float width;

    rand_f_max(POWER_UP_MIN_WIDTH, POWER_UP_MAX_WIDTH);
    pu->type = (rand_f() < 0) ? POWER_UP_TYPE_ACCELERATION : POWER_UP_TYPE_PADDLE_HEIGHT;
    pu->width = width;
    pu->height = width;
    pu->pos.x = rand_f_max(POWER_UP_MIN_X, POWER_UP_MAX_X);
    pu->pos.y = rand_f_max(POWER_UP_MIN_Y, POWER_UP_MAX_Y);
}

void init_balls(game_state *gs) {
    int i;
    ball *ball;

    gs->ball_count = BALL_INITIAL_COUNT;
    for (i = 0; i < gs->ball_count; i++)
        init_ball(&gs->balls[i]);
}

void end_game(game_state *gs, int status) {
    gs->end_time = clock();
    gs->status = status;
}

void restart_round(game_state *gs) {
    char i;

    reset_back_players(gs);
    if (gs->game_type == GAME_TYPE_2V2)
        reset_front_players(gs);
    init_balls(gs);
    init_power_ups(gs);
    sleep(GAME_RESTART_WAIT_TIME);
}

void update_velocity(vec2f *v, vec2f *a, float max_v_mod) {
    if (pow(pow(v->x + a->x, 2) + pow(v->y + a->y, 2), 0.5) <= max_v_mod)
        add_vec2f(v, a);
}

/* make sure than nothing goes out of the screen's borders */
/* maybe in some later implementation player might move on x axis as well */
void update_player(player *player, game_state *gs) {
    float fin_x, fin_y;
    float upper_lim, lower_lim, left_lim, right_lim;

    upper_lim = 0;
    lower_lim = GAME_WINDOW_HEIGHT - player->height;
    // left_lim = 0;
    // right_lim = WINDOW_WIDTH - player->width;

    update_velocity(&player->v, &player->a, PLAYER_MAX_VELOCITY_MOD);

    // fin_x = p->x + v->x;
    fin_y = player->pos.y + player->v.y;

    // if (fin_x < left_lim)
    //     p->x = left_lim;
    // else if (fin_x > right_lim)
    //     p->x = right_lim;
    // else
    //     p->x = fin_x;

    // check for walls
    if (fin_y < upper_lim)
        player->pos.y = upper_lim;
    else if (fin_y > lower_lim)
        player->pos.y = lower_lim;
    else
        player->pos.y = fin_y;
}

int check_for_collision(ball *ball, player *player) {
    int intersect_top, intersect_bottom, intersect_left, intersect_right;
    vec2f ball_final_pos = {ball->pos.x + ball->v.x, ball->pos.y + ball->v.y};
    // vec2f dv = {ball_final_pos.x - ball->pos.x, ball_final_pos.y - ball->pos.y};
    // double theta = atan(dv.x / dv.y);
    // ball_final_pos.x += sin(theta) * ball->radius;
    // ball_final_pos.y += cos(theta) * ball->radius;

    vec2f paddle_top_left_corner = {player->pos.x, player->pos.y};
    vec2f paddle_top_right_corner = {player->pos.x + player->width, player->pos.y};
    vec2f paddle_bottom_left_corner = {player->pos.x, player->pos.y + player->height};
    vec2f paddle_bottom_right_corner = {player->pos.x + player->width, player->pos.y + player->height};

    intersect_top = do_intersect(&ball->pos, &ball_final_pos, &paddle_top_left_corner, &paddle_top_right_corner);
    intersect_bottom = do_intersect(&ball->pos, &ball_final_pos, &paddle_bottom_left_corner, &paddle_bottom_right_corner);
    intersect_left = do_intersect(&ball->pos, &ball_final_pos, &paddle_top_left_corner, &paddle_bottom_left_corner);
    intersect_right = do_intersect(&ball->pos, &ball_final_pos, &paddle_top_right_corner, &paddle_bottom_right_corner);
    // if instersect top and ball above player => yvel *= (-1)
    if (intersect_top && ball->pos.y < player->pos.y){
        ball->a.y *= -1;
        ball->v.y *= -1;
        return 1;
    }
    if(intersect_right && player->pos.x + player->width < ball->pos.x){
        ball->a.x *= -1;
        ball->v.x *= -1;
        return 1;
    }
    if(intersect_bottom && ball->pos.y > player->pos.y + player->width){
        ball->a.y *= -1;
        ball->v.y *= -1;
        return 1;
    }
    if(intersect_left && ball->pos.x < player->pos.x){
        ball->a.x *= -1;
        ball->v.x *= -1;
        return 1;
    }



    /* ======================================== */

    // if ball x > paddle x + paddle_width and ball x >= paddle x + widthpp
    // float ball_left_x = ball->pos.x - ball->radius;
    // float ball_right_x = ball->pos.x + ball->radius;
    // float ball_upper_y = ball->pos.y - ball->radius;
    // float ball_lower_y = ball->pos.y + ball->radius;

    // float paddle_left_x = player->pos.x;
    // float paddle_right_x = player->pos.x + player->width;
    // float paddle_upper_y = player->pos.y;
    // float paddle_lower_y = player->pos.y + player->height;
    
    // float finx = ball->pos.x + ball->v.x;
    // float finy = ball->pos.y + ball->v.y;

    /* Checking for colision with player RIGHT SIDE */
    // if(ball_left_x <= paddle_right_x &&         // ball's left point is left to the paddle's right side or on it
    //     ball_right_x > paddle_left_x &&         // ball's right point is right to the paddle's left side
    //     ball_lower_y >= paddle_upper_y &&       // ball's lower point is under player's upper side
    //     ball_upper_y <= paddle_lower_y){        // ball's upper point is above player's lower side

    //     ball->v.x *= -1;
    //     ball->a.x *= -1;
    //     ball->pos.x = paddle_right_x + 2 + ball->radius;
    //     ball->pos.x += ball->v.x;
    //     ball->pos.y += ball->v.y;
    //     // ball->pos.x = player->pos.x + player->width + ball->radius;
    //     // ball->pos.y = finy;
    //     // ball->pos.x = paddle_right_x + 2;
    //     // ball->pos.y = finy;
    //     return 1;
    // }
    // /* Checking for colision with player LEFT SIDE */
    // if(ball_right_x >= paddle_left_x &&         // ball has touched the left side
    //     ball_left_x < paddle_right_x &&         // ball is left to right side
    //     ball_lower_y >= paddle_upper_y &&       // ball is below player top
    //     ball_upper_y <= paddle_lower_y) {       // ball is above player bottom
    
    //     ball->v.x *= -1;
    //     ball->a.x *= -1;
    //     ball->pos.x = paddle_left_x - ball->radius - 2;
    //     ball->pos.x += ball->v.x;
    //     ball->pos.y += ball->v.y;
    //     // ball->pos.x = paddle_left_x - ball->radius;
    //     // ball->pos.y = finy;
    //     return 1;
    // }
    // /* Checking for colision with player TOP */
    // if(ball_right_x >= paddle_left_x &&                      // ball is NOT left to player
    //     ball_left_x <= paddle_right_x &&                    // ball is NOT right to player
    //     ball_lower_y >= paddle_upper_y &&                   // ball is above player
    //     ball_upper_y < paddle_lower_y){                      // ball is NOT below player
    
    //     ball->v.y *= -1;
    //     ball->a.y *= -1;
    //     ball->pos.y = player->pos.y - ball->radius - 2;
    //     ball->pos.x += ball->v.x;
    //     ball->pos.y += ball->v.y;
    //     // ball->pos.x = finx;
    //     // ball->pos.y = paddle_upper_y - ball->radius;
    //     return 1;
    // }
    // /* Checking for colision with player BOTTOM */
    // if(ball_right_x >= paddle_left_x &&                      // ball is NOT left to player
    //     ball_left_x <= paddle_right_x &&                     // ball is NOT right to player
    //     ball_upper_y <= paddle_lower_y &&                    // ball is BELOW player
    //     ball_lower_y > paddle_upper_y){                      // ball is NOT above player

    //     ball->v.y *= -1;
    //     ball->a.y *= -1;
    //     ball->pos.y = player->pos.y + player->height + ball->radius + 2;
    //     ball->pos.x += ball->v.x;
    //     ball->pos.y += ball->v.y;
    //     // ball->pos.x = finx;
    //     // ball->pos.y = paddle_lower_y + ball->radius;
    //     return 1; 
    // } 


    return 0;
}

void init_power_ups(game_state *gs) {
    gs->power_up_count = POWER_UP_INITIAL_COUNT;
    power_up *p = &gs->power_ups[0];
    p->type = (rand_f() < 0) ? POWER_UP_TYPE_ACCELERATION : POWER_UP_TYPE_PADDLE_HEIGHT;
    float width = rand_f_max(POWER_UP_MIN_WIDTH, POWER_UP_MAX_WIDTH);
    p->width = width;
    p->height = width;
    p->pos.x = rand_f_max(POWER_UP_MIN_X, POWER_UP_MAX_X);
    p->pos.y = rand_f_max(POWER_UP_MIN_Y, POWER_UP_MAX_Y);
}
void add_radius_to_final_pos(vec2f *pos, vec2f *finpos, float radius) {
    vec2f trajectory = {finpos->x - pos->x, finpos->y - pos->y};
    double alpha = angle_with_vertical_vec2f(&trajectory) / (2 * M_PI);
    finpos->x += radius * sin(alpha);
    finpos->y += radius * cos(alpha);
}


void update_ball(ball *ball, game_state *gs) {
    player *player, *scored_player;
    team *t, *scored_team;
    vec2f *p, *v, *a;
    power_up *pu;
    float fin_x, fin_y;
    float upper_lim, lower_lim, left_lim, right_lim;
    char i;

    p = &ball->pos;
    v = &ball->v;
    a = &ball->a;

    upper_lim = ball->radius;
    lower_lim = GAME_WINDOW_HEIGHT - ball->radius;
    left_lim = ball->radius;
    right_lim = GAME_WINDOW_WIDTH - ball->radius;

    update_velocity(v, a, BALL_MAX_VELOCITY_MOD);

    fin_x = p->x + v->x;
    fin_y = p->y + v->y;

    // check for collisions with players
    for (i = 0; i < gs->player_count; i++) {
        player = &gs->players[i];
        if (player->client_id == PLAYER_DISCONNECTED_CLIENT_ID)
            continue;
        // if (check_for_collision(ball, player))
        //     goto _UPDATE_BALL_POWER_UPS;
        if ((fin_x + ball->radius >= player->pos.x) && (fin_x - ball->radius <= player->pos.x + player->width) &&
            (fin_y + ball->radius >= player->pos.y) && (fin_y - ball->radius <= player->pos.y + player->height)) {
            if (ball->pos.x  < player->pos.x) {
                ball->pos.x = player->pos.x - ball->radius;
                ball->pos.y = fin_y;
                ball->v.x *= -1;
                ball->a.x *= -1;
            }
            else if (ball->pos.x  > player->pos.x + player->width) {
                ball->pos.x = player->pos.x + player->width + ball->radius;
                ball->pos.y = fin_y;
                ball->v.x *= -1;
                ball->a.x *= -1;
            }
            else if (ball->pos.y  < player->pos.y) {
                ball->pos.x = fin_x;
                ball->pos.y = player->pos.y - ball->radius;
                ball->v.y *= -1;
                ball->a.y *= -1;
            }
            else if (ball->pos.y  > player->pos.y + player->height) {
                ball->pos.x = fin_x;
                ball->pos.y = player->pos.y + player->height + ball->radius;
                ball->v.y *= -1;
                ball->a.y *= -1;
            }
            ball->last_touched_id = player->id;
            goto _UPDATE_BALL_POWER_UPS; /* ball cant hit both wall and player in one frame */
        }
    }
    /* check for collisions with walls */
    if (fin_x < left_lim) {
        p->x = left_lim;
        v->x *= -1;
        a->x *= -1;
    }
    else if (fin_x > right_lim) {
        p->x = right_lim;
        v->x *= -1;
        a->x *= -1;
    }
    else
        ball->pos.x = fin_x;

    if (fin_y < upper_lim) {
        p->y = upper_lim;
        v->y *= -1;
        a->y *= -1;
    }
    else if (fin_y > lower_lim) {
        p->y = lower_lim;
        v->y *= -1;
        a->y *= -1;
    }
    else
        ball->pos.y = fin_y;

    _UPDATE_BALL_POWER_UPS:
    /* check for collision with power-ups */
    if (ball->last_touched_id != BALL_INITIAL_LAST_TOUCHED_ID) {
        for (i = 0; i < gs->power_up_count; i++) {
            pu = &gs->power_ups[i];
            if ((ball->pos.x + ball->radius >= pu->pos.x) && (ball->pos.x - ball->radius <= pu->pos.x + pu->width) &&
                (ball->pos.y + ball->radius >= pu->pos.y) && (ball->pos.y - ball->radius <= pu->pos.y + pu->height)) {
                switch (pu->type) {
                    case POWER_UP_TYPE_ACCELERATION:
                        /* accelerate ball until it hits enemy's paddle */
                        if (ball->v.x > 0)
                            ball->a.x = BALL_ACCELERATION_MOD;
                        else
                            ball->a.x = -BALL_ACCELERATION_MOD;

                        if (ball->v.y > 0)
                            ball->a.y = BALL_ACCELERATION_MOD;
                        else
                            ball->a.y = -BALL_ACCELERATION_MOD;
                        break;
                    case POWER_UP_TYPE_PADDLE_HEIGHT:
                        /* increase the height of the player who last hit the ball */
                        gs->players[ball->last_touched_id].height += 50;
                        break;
                    /* might add more */
                    default:
                        printf("Invalid power_up type (%d)\n", pu->type);
                }
                // remove power-up
                gs->power_up_count--;
                gs->power_ups[i].type = -1;
            }
        }
    }

    /* check for colision with team goal lines */
    for (i = 0; i < gs->team_count; i++) {
        t = &gs->teams[i];
        if (t->active_player_count == 0)
            continue;
        // !!! assuming that goal lines are ALWAYS vertical !!!
        if (mod_f(ball->pos.x - t->goal1.x) <= ball->radius) {
            if (ball->last_touched_id != BALL_INITIAL_LAST_TOUCHED_ID) {
                scored_player = &gs->players[ball->last_touched_id];
                scored_team = &gs->teams[scored_player->team_id];
                scored_player->score += 1;
            }
            else {
                /* !!! assuming that there are ALWAYS precisely two teams !!! */
                scored_team = (t->id == LEFT_TEAM_ID) ? &gs->teams[RIGHT_TEAM_ID] : &gs->teams[LEFT_TEAM_ID];
            }
            scored_team->score += 1;
            if (is_winning_team(scored_team, gs))
                end_game(gs, GAME_STATE_STATUS_SUCCESS);
            else
                restart_round(gs);
        }
    }
}

int is_winning_team(team *team, game_state *gs) {
    char i;
    struct _team *t;

    if (team->score >= TEAM_WINNING_SCORE_THRESHOLD)
        return 1;

    for (i = 0; i < gs->team_count; i++) {
        t = &gs->teams[i];
        if (t == team)
            continue;
        if (team->score - t->score >= TEAM_WINNING_SCORE_DIFFERENCE)
            return 1;
    } 
    return 0;
}

void reset_back_players(game_state *gs) {
    player *lp, *rp;

    lp = &gs->players[LEFT_BACK_PLAYER_ID];
    lp->pos.x = LEFT_BACK_PLAYER_INITIAL_X;
    lp->pos.y = LEFT_BACK_PLAYER_INITIAL_Y;
    lp->v.x = PLAYER_INITIAL_VELOCITY_X;
    lp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    lp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    lp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    lp->width = PLAYER_INITIAL_WIDTH;
    lp->height = PLAYER_INITIAL_HEIGHT;

    rp = &gs->players[RIGHT_BACK_PLAYER_ID];
    rp->pos.x = RIGHT_BACK_PLAYER_INITIAL_X;
    rp->pos.y = RIGHT_BACK_PLAYER_INITIAL_Y;
    rp->v.x = PLAYER_INITIAL_VELOCITY_X;
    rp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    rp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    rp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    rp->width = PLAYER_INITIAL_WIDTH;
    rp->height = PLAYER_INITIAL_HEIGHT;
}

void reset_front_players(game_state *gs) {
    player *lp, *rp;

    lp = &gs->players[LEFT_FRONT_PLAYER_ID];
    lp->pos.x = LEFT_FRONT_PLAYER_INITIAL_X;
    lp->pos.y = LEFT_FRONT_PLAYER_INITIAL_Y;
    lp->v.x = PLAYER_INITIAL_VELOCITY_X;
    lp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    lp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    lp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    lp->width = PLAYER_INITIAL_WIDTH;
    lp->height = PLAYER_INITIAL_HEIGHT;

    rp = &gs->players[RIGHT_FRONT_PLAYER_ID];
    rp->pos.x = RIGHT_FRONT_PLAYER_INITIAL_X;
    rp->pos.y = RIGHT_FRONT_PLAYER_INITIAL_Y;
    rp->v.x = PLAYER_INITIAL_VELOCITY_X;
    rp->v.y = PLAYER_INITIAL_VELOCITY_Y;
    rp->a.x = PLAYER_INITIAL_ACCELERATION_X;
    rp->a.y = PLAYER_INITIAL_ACCELERATION_Y;
    rp->width = PLAYER_INITIAL_WIDTH;
    rp->height = PLAYER_INITIAL_HEIGHT;
}

void reset_power_ups(game_state *gs) {
    char i;
    power_up *pu;

    for (i = 0; i < gs->power_up_count; i++) {
        pu = &gs->power_ups[i];
        // TODO: reset
    }
}

int all_players_ready(game_state *gs) {
    char i;

    for (i = 0; i < gs->player_count; i++) {
        if (gs->players[i].ready == PLAYER_READY_FALSE)
            return 0;
    }
    return 1;
}

int all_players_disconnected(game_state *gs) {
    int i;

    for (i = 0; i < gs->player_count; i++) {
        if (gs->players[i].client_id != PLAYER_DISCONNECTED_CLIENT_ID)
            return 0;
    }
    return 1;
}

double time_diff_in_seconds(clock_t t1, clock_t t2) {
    return (double) (t2 - t1) / CLOCKS_PER_SEC;
}


int on_segment(vec2f *p, vec2f *q, vec2f *r) {
    return (q->x <= max(p->x, r->x) && q->x >= min(p->x, r->x) &&
            q->y <= max(p->y, r->y) && q->y >= min(p->y, r->y));
}

int orientation(vec2f *p, vec2f *q, vec2f *r) {
    int val;

    val = (q->y - p->y) * (r->x - q->x) -
            (q->x - p->x) * (r->y - q->y);
    if (val == 0)
        return 0;
    return (val > 0) ? 1 : 2;
}

int do_intersect(vec2f *p1, vec2f *q1, vec2f *p2, vec2f *q2) {
    int o1, o2, o3, o4;

    o1 = orientation(p1, q1, p2);
    o2 = orientation(p1, q1, q2);
    o3 = orientation(p2, q2, p1);
    o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return 1;

    if (o1 == 0 && on_segment(p1, p2, q1))
        return 1;
    if (o2 == 0 && on_segment(p1, q2, q1))
        return 1;
    if (o3 == 0 && on_segment(p2, p1, q2))
        return 1;
    if (o4 == 0 && on_segment(p2, q1, q2))
        return 1;

    return 0;
}



void print_team(team *team) {
    printf("id: %d\n", team->id);
    printf("score: %d\n", team->score);
    printf("goal1: (%f, %f)\n", team->goal1.x, team->goal1.y);
    printf("goal2: (%f, %f)\n", team->goal2.x, team->goal2.y);
}

void print_player(player *player) {
    printf("id: %d\n", player->id);
    printf("client_id: %d\n", player->client_id);
    printf("team_id: %d\n", player->team_id);
    printf("ready: %d\n", player->ready);
    printf("name: %s\n", player->name);
    printf("score: %d\n", player->score);
    printf("pos: (%f, %f)\n", player->pos.x, player->pos.y);
    printf("v: (%f, %f)\n", player->v.x, player->v.y);
    printf("a: (%f, %f)\n", player->a.x, player->a.y);
    printf("width: %f\n", player->width);
    printf("height: %f\n", player->height);
}

void print_ball(ball *ball) {
    printf("pos: (%f, %f)\n", ball->pos.x, ball->pos.y);
    printf("v: (%f, %f)\n", ball->v.x, ball->v.y);
    printf("a: (%f, %f)\n", ball->a.x, ball->a.y);
    printf("radius: %f\n", ball->radius);
    printf("type: %d\n", ball->power_up->type);
    printf("last_touched_id: %d\n", ball->last_touched_id);
}

void print_power_up(power_up *power_up) {
    printf("pos: (%f, %f)\n", power_up->pos.x, power_up->pos.y);
    printf("width: %f\n", power_up->width);
    printf("height: %f\n", power_up->height);
    printf("type: %d\n", power_up->type);
}

void print_game_state(game_state *gs) {
    char i;

    printf("--GENERAL INFO--\n");
    printf("last_update: %lu\n", gs->last_update);
    printf("start_time: %lu\n", gs->start_time);
    printf("end_time: %lu\n", gs->end_time);
    printf("status: %d\n", gs->status);
    printf("game_type: %d\n", gs->game_type);
    printf("window_width: %d\n", gs->window_width);
    printf("window_height: %d\n", gs->window_height);

    putchar('\n');
    printf("--TEAMS--\n");
    printf("team_count: %d\n", gs->team_count);
    putchar('\n');
    for (i = 0; i < gs->team_count; i++) {
        print_team(&gs->teams[i]);
        putchar('\n');
    }

    putchar('\n');
    printf("--PLAYERS--\n");
    printf("player_count: %d\n", gs->player_count);
    putchar('\n');
    for (i = 0; i < gs->player_count; i++) {
        print_player(&gs->players[i]);
        putchar('\n');
    }

    putchar('\n');
    printf("--BALLS--\n");
    printf("ball_count: %d\n", gs->ball_count);
    putchar('\n');
    for (i = 0; i < gs->ball_count; i++) {
        print_ball(&gs->balls[i]);
        putchar('\n');
    }

    putchar('\n');
    printf("--POWER_UPS--\n");
    putchar('\n');
    for (i = 0; i < gs->power_up_count; i++) {
        print_power_up(&gs->power_ups[i]);
        putchar('\n');
    }
}