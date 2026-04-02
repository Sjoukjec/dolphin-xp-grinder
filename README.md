# Dolphin XP Grinder 🐬⚡

**A Flipper Zero app that levels up your Dolphin by running animated "hack" sequences on screen.**

<img width="1376" height="768" alt="flipper_zero_pro" src="https://github.com/user-attachments/assets/86e70cbd-9154-46d3-b31f-c37af02da733" />


No radios. No cards. No external devices. Just your Dolphin eating XP like it's pizza. Think of it as a cookie clicker for your little digital companion.

---

## The Philosophy

> *"I love 'useless projects' that do nothing but work a system in a way it was designed but not intended. That type of process turned me into who I am today."*
> — u/GuidoZ, r/flipperzero Community Expert

> *"Not everything needs to be some essential, must-have tool. It's about going beyond the basics, experimenting, and building something just because you can!"*
> — u/VVr3nch, r/flipperzero Community Manager

This project exists because **playing is the best way of learning**. The Flipper Zero is a tool for tinkering, curiosity, and understanding how things work — and that includes the Flipper *itself*.

Building this taught me:
- How the Dolphin gamification system works under the hood
- How `dolphin_deed()` distributes XP across activity buckets
- How to write a Flipper app from scratch (ViewPort, input handling, tick-based animation)
- How the FAP build system works

Could you just edit a save file? Sure. But where's the fun in that?

**Nothing more fun than hacking the hacking device.** 🤷

---

## What It Does

Runs 7 terminal-style animated sequences. Each one plays out like a mini movie on the 128×64 OLED, then fires the real Dolphin deed to award XP. Haptic buzz on completion because *feedback matters*.

| Sequence | What It "Does" | XP |
|----------|---------------|----|
| Sub-GHz Attack | Pretends to capture a signal | 3 |
| RFID Clone | Pretends to duplicate a badge | 3 |
| NFC Jackpot | Pretends to dump a card | 3 |
| IR Domination | Pretends to learn a remote | 3 |
| iButton Dupe | Pretends to clone a key | 3 |
| BadUSB Nuke | Pretends to deploy a payload | 3 |
| Game Win | Pretends you won a game (because you did) | 10 |

**+28 XP per complete run.** The Dolphin system enforces daily XP caps — this app respects them. Run it, come back tomorrow, repeat.

---

## Controls

| Button | Action |
|--------|--------|
| **OK** | Start / Skip ahead / Run again |
| **Back** | Abort to summary / Exit |

---

## Install

### Drop the FAP (easiest)

1. Grab `hacker_sim.fap` from [Releases](../../releases)
2. Copy to your SD card: `/ext/apps/Games/hacker_sim.fap`
3. On device: **Apps → Games → Hacker Sim**

### Build It Yourself (recommended — you'll learn stuff)

```bash
# Install ufbt (Flipper's micro build tool)
pip install ufbt

# Clone and build
git clone https://github.com/Sjoukjec/dolphin-xp-grinder.git
cd dolphin-xp-grinder
ufbt build
```

Output lands in `dist/`. Copy the `.fap` to your Flipper.

### Or drop it into a firmware tree

Copy `hacker_sim.c` and `application.fam` into `applications_user/hacker_sim/`, then:

```bash
./fbt launch APPSRC=applications_user/hacker_sim
```

---

## How It Works (for the curious)

The entire app is ~350 lines of C. It uses only stock Flipper SDK APIs:

- **`dolphin_deed()`** — the same function every built-in app calls when you successfully do something (read a card, capture a signal, etc.). We're using the official API, not a hack.
- **`gui/gui.h`** — draws text and lines on the 128×64 OLED.
- **`notification/notification.h`** — vibrates the motor for haptic feedback.

**Zero radio activity. Zero external device interaction.** The terminal text is purely cosmetic animation. The XP is real.

If you want to understand how it all fits together, read the source — it's well-structured and commented. Steal patterns from it for your own apps. That's the point.

---

## FAQ

**Does this actually hack anything?**
Nope. No radio, no scanning, no external interaction. Screen animations + Dolphin API calls.

**Will this break my Flipper?**
No. The Dolphin has built-in daily XP caps. Nothing overflows, nothing corrupts.

**Works on stock firmware?**
Yes. Standard SDK APIs only.

**Can I get banned?**
The Dolphin is a local gamification system on *your* device. No server, no leaderboard, no online component. It's your Dolphin. Feed it however you want.

**Why not just edit the save file?**
You could. But you wouldn't learn anything, and your Dolphin would know you cheated. 🐬

---

## Build Your Own Silly Thing

The best Flipper projects aren't the "useful" ones. They're the ones where someone asked *"what if..."* and just went for it. If this inspires you to build something equally pointless and fun — mission accomplished.

Fork it. Break it. Make it play music. Add a Konami code. Whatever. That's the spirit.

---

## License

MIT — see [LICENSE](LICENSE)

---

*"It's about learning, tinkering, working the system. That's the point of the device."* 🐬
