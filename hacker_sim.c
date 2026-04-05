#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <dolphin/dolphin.h>
#include <dolphin/helpers/dolphin_deed.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#define TAG              "HackerSim"
#define TICK_MS          700UL
#define RESULT_LINGER_MS 1000UL

typedef enum {
    SimStateIntro,
    SimStateHacking,
    SimStateDone,
} SimState;

typedef struct {
    const char* title;
    DolphinDeed deed;
    uint8_t xp;
    const char* lines[4];
} HackSeq;

static const HackSeq sequences[] = {
    {
        "SUB-GHZ READ",
        DolphinDeedSubGhzSave,
        3,
        {
            "> Tuning 433.920MHz...",
            "> Signal acquired!",
            "> RAW capture done",
            ">> LOCKED & SAVED",
        },
    },
    {
        "RFID CLONE",
        DolphinDeedRfidReadSuccess,
        3,
        {
            "> 125kHz field: ON",
            "> HID card detected!",
            "> Cloning sectors...",
            ">> BADGE DUPLICATED",
        },
    },
    {
        "NFC JACKPOT",
        DolphinDeedNfcReadSuccess,
        3,
        {
            "> NFC field active",
            "> Mifare Classic!",
            "> Dumping 1K...",
            ">> CARD CLONED",
        },
    },
    {
        "IR DOMINATION",
        DolphinDeedIrLearnSuccess,
        3,
        {
            "> IR capture: ON",
            "> Signal received!",
            "> Samsung32 proto",
            ">> REMOTE OWNED",
        },
    },
    {
        "iBUTTON DUPE",
        DolphinDeedIbuttonReadSuccess,
        3,
        {
            "> 1-Wire bus active",
            "> DS1990A found!",
            "> Reading ROM...",
            ">> KEY DUPLICATED",
        },
    },
    {
        "BADUSB NUKE",
        DolphinDeedBadUsbPlayScript,
        3,
        {
            "> HID enum: DONE",
            "> Host accepted!",
            "> Payload running...",
            ">> SCRIPT DEPLOYED",
        },
    },
    {
        "GAME WIN",
        DolphinDeedPluginGameWin,
        10,
        {
            "> Loading Mayhem.exe",
            "> Enemy neutralized!",
            "> Score: 9001",
            ">> YOU WIN! +10XP",
        },
    },
};

#define SEQ_COUNT ((uint8_t)(sizeof(sequences) / sizeof(sequences[0])))

typedef struct {
    SimState state;
    uint8_t seq_idx;
    uint8_t visible_lines; // 0-4: how many terminal lines are shown
    bool deed_fired;       // deed for current seq has been called
    bool result_shown;     // lingering on result before advancing
    uint32_t session_xp;
    uint32_t last_tick_ms;
    ViewPort* view_port;
    FuriMessageQueue* input_queue;
    NotificationApp* notifications;
    bool exit;
} HackerSim;

// ─── Rendering ────────────────────────────────────────────────────────────────

static void hacker_sim_draw_intro(Canvas* canvas) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 8, "MAYHEM HACKER SIM");
    canvas_draw_line(canvas, 0, 10, 127, 10);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 20, "> Initializing subsystems");
    canvas_draw_str(canvas, 2, 30, "> XP farmer: LOADED");
    canvas_draw_str(canvas, 2, 40, "> Dolphin: hungry af");
    canvas_draw_str(canvas, 2, 50, "> Ready to cause mayhem");

    canvas_draw_line(canvas, 0, 53, 127, 53);
    canvas_draw_str(canvas, 2, 63, "[OK] hack  [Back] exit");
}

static void hacker_sim_draw_hacking(Canvas* canvas, HackerSim* sim) {
    const HackSeq* seq = &sequences[sim->seq_idx];

    // Header
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 8, seq->title);
    if(sim->deed_fired) {
        char xp_str[10];
        snprintf(xp_str, sizeof(xp_str), "+%d XP!", seq->xp);
        if(seq->xp < 10) {
            canvas_draw_str(canvas, 93, 8, xp_str);
        }
        else {
            canvas_draw_str(canvas, 87, 8, xp_str);
        }
    }
    canvas_draw_line(canvas, 0, 10, 127, 10);

    // Terminal lines (appear one-by-one)
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < sim->visible_lines && i < 4; i++) {
        canvas_draw_str(canvas, 2, 20 + (int8_t)(i * 10), seq->lines[i]);
    }

    // Footer
    canvas_draw_line(canvas, 0, 53, 127, 53);
    char footer[32];
    snprintf(footer, sizeof(footer), "Session: +%lu XP", (unsigned long)sim->session_xp);
    canvas_draw_str(canvas, 2, 63, footer);
    canvas_draw_str(canvas, 91, 63, "[OK]skip");
}

static void hacker_sim_draw_done(Canvas* canvas, HackerSim* sim) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 8, "SESSION COMPLETE");
    canvas_draw_line(canvas, 0, 10, 127, 10);

    canvas_set_font(canvas, FontSecondary);

    char buf[32];
    snprintf(buf, sizeof(buf), "> %d hacks executed", SEQ_COUNT);
    canvas_draw_str(canvas, 2, 20, buf);

    snprintf(buf, sizeof(buf), "> XP earned: +%lu", (unsigned long)sim->session_xp);
    canvas_draw_str(canvas, 2, 30, buf);

    canvas_draw_str(canvas, 2, 40, "> Dolphin: leveling up");
    canvas_draw_str(canvas, 2, 50, "> Status: MAXIMUM EFFORT");

    canvas_draw_line(canvas, 0, 53, 127, 53);
    canvas_draw_str(canvas, 2, 63, "[OK] again  [Back] quit");
}

static void hacker_sim_render(Canvas* canvas, void* ctx) {
    HackerSim* sim = ctx;
    canvas_clear(canvas);

    switch(sim->state) {
    case SimStateIntro:
        hacker_sim_draw_intro(canvas);
        break;
    case SimStateHacking:
        hacker_sim_draw_hacking(canvas, sim);
        break;
    case SimStateDone:
        hacker_sim_draw_done(canvas, sim);
        break;
    }
}

// ─── Input ────────────────────────────────────────────────────────────────────

static void hacker_sim_input(InputEvent* event, void* ctx) {
    HackerSim* sim = ctx;
    furi_message_queue_put(sim->input_queue, event, 0);
}

// ─── Tick / Auto-advance ──────────────────────────────────────────────────────

static void hacker_sim_start_sequence(HackerSim* sim) {
    sim->seq_idx = 0;
    sim->visible_lines = 0;
    sim->deed_fired = false;
    sim->result_shown = false;
    sim->state = SimStateHacking;
    sim->last_tick_ms = furi_get_tick();
}

static void hacker_sim_tick(HackerSim* sim) {
    if(sim->state != SimStateHacking) return;

    if(sim->visible_lines < 4) {
        // Reveal next line
        sim->visible_lines++;

        if(sim->visible_lines == 4 && !sim->deed_fired) {
            // All lines visible: fire the deed
            dolphin_deed(sequences[sim->seq_idx].deed);
            sim->session_xp += sequences[sim->seq_idx].xp;
            sim->deed_fired = true;
            notification_message(sim->notifications, &sequence_single_vibro);
        }
    } else if(!sim->result_shown) {
        // Linger one extra tick on the result so the XP flash is readable
        sim->result_shown = true;
    } else {
        // Advance to next sequence
        sim->seq_idx++;
        if(sim->seq_idx >= SEQ_COUNT) {
            sim->state = SimStateDone;
        } else {
            sim->visible_lines = 0;
            sim->deed_fired = false;
            sim->result_shown = false;
        }
    }

    view_port_update(sim->view_port);
}

// Skip ahead: immediately fire deed and jump to next seq
static void hacker_sim_skip(HackerSim* sim) {
    if(sim->state == SimStateIntro) {
        hacker_sim_start_sequence(sim);
        view_port_update(sim->view_port);
        return;
    }

    if(sim->state == SimStateDone) {
        sim->session_xp = 0;
        hacker_sim_start_sequence(sim);
        view_port_update(sim->view_port);
        return;
    }

    if(sim->state == SimStateHacking && !sim->deed_fired) {
        sim->visible_lines = 4;
        dolphin_deed(sequences[sim->seq_idx].deed);
        sim->session_xp += sequences[sim->seq_idx].xp;
        sim->deed_fired = true;
        sim->result_shown = true;
        notification_message(sim->notifications, &sequence_single_vibro);
        sim->last_tick_ms = furi_get_tick();
        view_port_update(sim->view_port);
    }
}

// ─── Entry point ──────────────────────────────────────────────────────────────

int32_t hacker_sim_app(void* p) {
    UNUSED(p);

    HackerSim* sim = malloc(sizeof(HackerSim));
    sim->state = SimStateIntro;
    sim->seq_idx = 0;
    sim->visible_lines = 0;
    sim->deed_fired = false;
    sim->result_shown = false;
    sim->session_xp = 0;
    sim->exit = false;
    sim->last_tick_ms = furi_get_tick();

    sim->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    sim->view_port = view_port_alloc();
    view_port_draw_callback_set(sim->view_port, hacker_sim_render, sim);
    view_port_input_callback_set(sim->view_port, hacker_sim_input, sim);

    sim->notifications = furi_record_open(RECORD_NOTIFICATION);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, sim->view_port, GuiLayerFullscreen);

    // Register with dolphin so plugin deed bucket is credited correctly
    dolphin_deed(DolphinDeedPluginStart);

    view_port_update(sim->view_port);

    while(!sim->exit) {
        // Handle input (non-blocking)
        InputEvent event;
        if(furi_message_queue_get(sim->input_queue, &event, 10) == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                if(event.key == InputKeyBack) {
                    if(sim->state == SimStateIntro || sim->state == SimStateDone) {
                        sim->exit = true;
                    } else {
                        // Abort current run → show summary
                        sim->state = SimStateDone;
                        view_port_update(sim->view_port);
                    }
                } else if(event.key == InputKeyOk) {
                    hacker_sim_skip(sim);
                }
            }
        }

        // Auto-advance tick
        if(sim->state == SimStateHacking) {
            uint32_t now = furi_get_tick();
            uint32_t interval = (sim->deed_fired && !sim->result_shown) ? RESULT_LINGER_MS : TICK_MS;
            if(now - sim->last_tick_ms >= interval) {
                sim->last_tick_ms = now;
                hacker_sim_tick(sim);
            }
        }
    }

    gui_remove_view_port(gui, sim->view_port);
    view_port_free(sim->view_port);
    furi_message_queue_free(sim->input_queue);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    free(sim);

    return 0;
}
