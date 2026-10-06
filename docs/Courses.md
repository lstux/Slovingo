# Slovingo Course Instances

This page tracks the Slovingo language course repositories and their deployment status.

## Available Courses

| Course | Repository | Visibility | lang.json | Last Updated |
|--------|------------|------------|-----------|--------------|
| Slovak (sk → fr) | [Slovingo-sk-fr](https://github.com/lstux/Slovingo-sk-fr) | Public | ✓ | 2026-10-05 |
| Slovak Kids (8–12, sk → fr) | [Slovingo-sk-fr-kids](https://github.com/lstux/Slovingo-sk-fr-kids) | Public | ✓ | 2026-10-06 |
| Slovak Teens (12–16, sk → fr) | [Slovingo-sk-fr-friends](https://github.com/lstux/Slovingo-sk-fr-friends) | Public | ✓ | 2026-10-05 |
| French (fr → sk) | [Slovingo-fr-sk](https://github.com/lstux/Slovingo-fr-sk) | Public | ✓ | 2026-09-29 |
| German (de → fr) | [Slovingo-de-fr](https://github.com/lstux/Slovingo-de-fr) | Public | ✓ | 2026-10-05 |
| German Kids (8–12, de → fr) | [Slovingo-de-fr-kids](https://github.com/lstux/Slovingo-de-fr-kids) | Public | ✓ | 2026-10-05 |
| Breton (bzh → fr) | [Slovingo-bzh-fr](https://github.com/lstux/Slovingo-bzh-fr) | Public | ✓ | 2026-09-29 |

## In Preparation

| Course | Repository | Status |
|--------|------------|--------|
| French Kids (8–12, fr → sk) | [Slovingo-fr-sk-kids](https://github.com/lstux/Slovingo-fr-sk-kids) | ⏳ Not yet created |

---

## Column Definitions

- **Repository**: GitHub repo link
- **Visibility**: Public or Private
- **lang.json**: ✓ = config file present and current; ✗ = missing or outdated
- **Last Updated**: Date of most recent commit

## Adding a New Course

Each course is a separate repository following the `Slovingo-<language>-<variant>` naming convention:
- Clone the core [Slovingo](https://github.com/lstux/Slovingo) repository
- Create a new `langs/<lang>-<native>` directory with `.md` source files, images, and `lang.json` config
- Build with `python3 src/build.py --lang-dir langs/<lang>-<native>`
- Push to a new repo on GitHub

See the main repository's `README.md` for the full workflow.
