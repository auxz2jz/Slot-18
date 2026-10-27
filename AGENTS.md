# AGENTS.md — Slot-18 development entry point
Read and obey the **current canonical** master library at https://github.com/auxz2jz/master-instruction-library. In order:
1. INSTRUCTION_INDEX.md
2. CORE_DEVELOPMENT_RECOVERY_RULES.md
3. DIAGNOSTICS_STANDARD.md
4. GUIDED_TESTING_STANDARD.md
5. CODEX_USAGE_EFFICIENCY_STANDARD.md
6. CROSS_PLATFORM_COLLABORATION_STANDARD.md when multiple platforms are built
Then read this repo's PROJECT_MEMORY.md, ROADMAP.md, docs/ARCHITECTURE.md and docs/DIAGNOSTICS_AND_TESTING.md BEFORE changing code.

Project: handheld 7-inch ESP32-S3 touchscreen + modular radar and non-radar sensor system, up to 5+ battery remote stations and eventually four 3-radar direction groups. Browser first version remains simulated; ESP-IDF application firmware is unverified. **As of 2026-10-27, actual Waveshare factory screen/touch IS user-verified**, CJMCU-752 I²C and internal dual-UART loopbacks passed Arduino diagnostics, but C4002 external UART and radar readings are NOT verified. Start with root `NEXT_CHAT_HANDOFF.md` and `docs/2026-10-27_PHYSICAL_BENCH_CHECKPOINT.md`; previous no-hardware assertions are historical.

Hard constraints:
- Do not call simulator measurements live, and do not plot presence-only observations as precise target dots.
- Keep a hardware-verified baseline separate from a browser-simulator candidate.
- Do not assume all four/eight sectors or 24 concurrently transmitting 24GHz sensors are interference-free.
- No automatic actual human identification or reliable wall/floor penetration claims.
- Preserve diagnostics, guided user tests, changelog/status and recovery steps. Record observed results, avoid unrelated edits.
- Prefer Waveshare official display LVGL/ESP-IDF sample for hardware bring-up, then adapt prototype core.
- Use economical staged implementation and tests per master efficiency rule.
- Do not make repository changes beyond this project's scope or edit other Slot repositories without explicit direction.
