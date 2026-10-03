/**
 * celebrate.js -- end-of-session celebration effect (pure JS + canvas).
 *
 * One public function, called from renderSummary() in exercises.js:
 *
 *     celebrate(correct, total, anchorElement)
 *
 * The intensity depends on the score ratio (same thresholds as
 * scoreRatioClass() in progress.js):
 *
 *     0 %             "misfire"   the fuse crackles, the rocket hops and gives up
 *     1 - 49 %        "dud"       three rockets: one fizzles out, one "explodes"
 *                                 into three sad confetti, one goes pop
 *     50 - 79 %       "sparkle"   a small, discreet burst of glitter
 *     80 - 99 %       "confetti"  two confetti cannons
 *     100 %           "fireworks" rockets + finale confetti
 *
 * Fireworks need at least MIN_FOR_FIREWORKS exercises: a perfect 3/3
 * only earns the confetti.
 *
 * Respects prefers-reduced-motion (no effect at all). The canvas is
 * pointer-events: none, aria-hidden, and removes itself when the last
 * particle is gone, when the route changes, or after a hard time limit.
 *
 * Exposes window.celebrate and window.stopCelebration.
 */
(function () {
    "use strict";

    const MIN_FOR_FIREWORKS = 5;
    const HARD_STOP_MS = 8000;
    const MAX_PARTICLES = 450;
    const GRAVITY = 900;            // px/s^2
    const TRAIL_LENGTH = 7;         // positions kept for spark / rocket trails

    // Rocket body and trail: amber stays visible on light and dark themes.
    const ROCKET_COLOR = "#f59e0b";

    const FALLBACK_COLORS = ["#ffc83d", "#ff5d8f", "#4cc9f0", "#7bd88f", "#b794f6", "#ff8a4c"];

    let canvas = null;
    let ctx = null;
    let rafId = 0;
    let hardStopId = 0;
    let lastTime = 0;
    let particles = [];
    let rockets = [];
    let spawnQueue = [];            // { at: ms, fn }
    let startedAt = 0;
    let width = 0;
    let height = 0;
    let dpr = 1;

    // ------------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------------

    function reducedMotion() {
        return !!(window.matchMedia && window.matchMedia("(prefers-reduced-motion: reduce)").matches);
    }

    function rand(min, max) {
        return min + Math.random() * (max - min);
    }

    function pick(array) {
        return array[Math.floor(Math.random() * array.length)];
    }

    /**
     * Resolve --accent / --accent-2 to real colors. The theme variables
     * use light-dark(), which getPropertyValue() returns unresolved, so
     * we let the browser compute a color on a throwaway element.
     */
    function themeColors() {
        const colors = [];
        const probe = document.createElement("span");
        probe.style.display = "none";
        document.body.appendChild(probe);
        ["--accent", "--accent-2"].forEach((name) => {
            probe.style.color = `var(${name})`;
            const value = getComputedStyle(probe).color;
            if (value) colors.push(value);
        });
        probe.remove();
        return colors.concat(FALLBACK_COLORS);
    }

    function ensureCanvas() {
        if (canvas) return;
        canvas = document.createElement("canvas");
        canvas.className = "celebration-canvas";
        canvas.setAttribute("aria-hidden", "true");
        document.body.appendChild(canvas);
        ctx = canvas.getContext("2d");
        resize();
        window.addEventListener("resize", resize);
    }

    function resize() {
        if (!canvas) return;
        dpr = Math.min(window.devicePixelRatio || 1, 2);
        width = window.innerWidth;
        height = window.innerHeight;
        canvas.width = Math.round(width * dpr);
        canvas.height = Math.round(height * dpr);
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    }

    function addParticle(p) {
        if (particles.length >= MAX_PARTICLES) return;
        particles.push(p);
    }

    // ------------------------------------------------------------------
    // Particle factories
    // ------------------------------------------------------------------

    /** A glitter dot: small, fast, twinkles, fades. */
    function glitter(x, y, angle, speed, color, life, opts = {}) {
        addParticle({
            kind: "dot", x, y,
            vx: Math.cos(angle) * speed, vy: Math.sin(angle) * speed,
            drag: 1.6, gravity: opts.gravity ?? GRAVITY * 0.35,
            size: opts.size ?? rand(1.5, 3.2), color, life, age: 0,
            twinkle: rand(8, 18),
        });
    }

    /**
     * A paper confetti: rotating rectangle with flutter.
     * opts: gravity (px/s^2), big (bigger piece, for the sad volley), life.
     */
    function confettiPiece(x, y, angle, speed, color, opts = {}) {
        addParticle({
            kind: "paper", x, y,
            vx: Math.cos(angle) * speed, vy: Math.sin(angle) * speed,
            drag: 1.1, gravity: opts.gravity ?? GRAVITY * 0.55,
            w: opts.big ? rand(10, 15) : rand(6, 11),
            h: opts.big ? rand(5, 8) : rand(3, 6),
            rot: rand(0, Math.PI * 2), vrot: rand(-9, 9),
            flip: rand(0, Math.PI * 2), vflip: rand(4, 10),
            color, life: opts.life ?? rand(2.6, 4.2), age: 0,
        });
    }

    /** A puff of grey smoke: grows, drifts up, fades. */
    function smoke(x, y, size, life) {
        addParticle({
            kind: "smoke", x, y,
            vx: rand(-12, 12), vy: -rand(14, 30),
            drag: 0.8, gravity: -10,
            size, color: "#9a9a9a", life, age: 0,
        });
    }

    /** A firework spark, leaves a short trail through canvas fading. */
    function spark(x, y, angle, speed, color) {
        addParticle({
            kind: "dot", x, y,
            vx: Math.cos(angle) * speed, vy: Math.sin(angle) * speed,
            drag: 1.8, gravity: GRAVITY * 0.28,
            size: rand(1.6, 2.6), color, life: rand(0.9, 1.5), age: 0,
            twinkle: 0, trail: true, hist: [],
        });
    }

    // ------------------------------------------------------------------
    // Effects
    // ------------------------------------------------------------------

    function effectSparkle(origin, colors) {
        const gold = ["#ffd86b", "#fff1b8", colors[0], colors[1]];
        const count = 38;
        for (let i = 0; i < count; i++) {
            glitter(origin.x, origin.y, rand(0, Math.PI * 2), rand(60, 260), pick(gold), rand(0.8, 1.5));
        }
    }

    function effectConfetti(colors) {
        // Two cannons at the bottom corners, aimed at the middle.
        const burst = (x, baseAngle) => {
            for (let i = 0; i < 55; i++) {
                confettiPiece(
                    x, height + 10,
                    baseAngle + rand(-0.38, 0.38),
                    rand(620, 1050),
                    pick(colors)
                );
            }
        };
        burst(-10, -Math.PI * 0.30);
        burst(width + 10, -Math.PI * 0.70);
        // Second, softer volley shortly after.
        spawnQueue.push({
            at: 380,
            fn: () => {
                burst(-10, -Math.PI * 0.26);
                burst(width + 10, -Math.PI * 0.74);
            },
        });
    }

    /**
     * Rocket modes:
     *   "normal"  climbs high and explodes properly (fireworks)
     *   "fizzle"  wobbles, runs out of steam, drips a few sparks
     *   "weak"    "explodes" into three sad confetti pieces
     *   "pop"     a tiny pop: two sparks and a puff of smoke
     *   "misfire" hops a few pixels off the ground and gives up
     * `over` overrides any field of the rocket (position, speed...).
     */
    function launchRocket(colors, mode = "normal", over = {}) {
        const rocket = {
            mode, colors, hist: [],
            x: rand(width * 0.15, width * 0.85),
            y: height + 5,
            vx: rand(-40, 40),
            vy: -rand(height * 0.9, height * 1.25),
            targetY: rand(height * 0.15, height * 0.45),
            color: pick(colors),
            wobble: false,
        };
        if (mode === "fizzle") {
            rocket.vx = 0;
            rocket.vy = -rand(height * 0.62, height * 0.75);
            rocket.targetY = rand(height * 0.48, height * 0.58);
            rocket.wobble = true;
        } else if (mode === "weak") {
            rocket.vx = rand(-15, 15);
            rocket.vy = -rand(height * 0.85, height * 1.0);
            rocket.targetY = rand(height * 0.30, height * 0.40);
        } else if (mode === "pop") {
            rocket.vx = rand(-15, 15);
            rocket.vy = -rand(height * 0.7, height * 0.8);
            rocket.targetY = rand(height * 0.55, height * 0.65);
        } else if (mode === "misfire") {
            rocket.vx = 0;
            rocket.vy = -rand(200, 240);
            rocket.targetY = 0;
        }
        rockets.push(Object.assign(rocket, over));
    }

    function explode(rocket) {
        const count = 70 + Math.floor(rand(0, 30));
        const main = rocket.color;
        const alt = pick(rocket.colors);
        const speed = rand(180, 340);
        for (let i = 0; i < count; i++) {
            const angle = (i / count) * Math.PI * 2 + rand(-0.05, 0.05);
            // Slightly varied radius gives a fuller, less mechanical ball.
            const s = speed * rand(0.55, 1);
            spark(rocket.x, rocket.y, angle, s, i % 5 === 0 ? alt : main);
        }
    }

    /** Run a function `delayMs` from now (relative to the running animation). */
    function later(delayMs, fn) {
        spawnQueue.push({ at: performance.now() - startedAt + delayMs, fn });
    }

    /** A few weak embers dripping down from a spent rocket. */
    function drips(x, y, count) {
        const embers = ["#e08a2c", "#b9722a", "#d9a441"];
        for (let i = 0; i < count; i++) {
            glitter(x + rand(-4, 4), y, rand(0.4, Math.PI - 0.4), rand(15, 55), pick(embers),
                rand(0.7, 1.1), { gravity: GRAVITY * 0.9, size: rand(1.6, 2.6) });
        }
    }

    /** What happens when a rocket reaches the top of its climb. */
    function detonate(rocket) {
        const { x, y } = rocket;
        switch (rocket.mode) {
        case "fizzle":
            drips(x, y, 4);
            smoke(x, y, 7, 1.3);
            break;
        case "weak":
            for (let i = 0; i < 3; i++) {
                confettiPiece(x, y, -Math.PI / 2 + rand(-1.3, 1.3), rand(30, 90), pick(rocket.colors),
                    { gravity: GRAVITY * 1.1, big: true, life: 2.4 });
            }
            smoke(x, y, 6, 1.1);
            break;
        case "pop":
            for (let i = 0; i < 2; i++) {
                spark(x, y, rand(0, Math.PI * 2), rand(40, 90), "#ffd86b");
            }
            smoke(x, y, 10, 1.7);
            break;
        case "misfire":
            drips(x, y, 2);
            smoke(x, y, 8, 1.6);
            later(250, () => smoke(x + rand(-6, 6), y - 6, 6, 1.2));
            break;
        default:
            explode(rocket);
        }
    }

    /** Score below 50 %: rockets go up... and it doesn't go very well. */
    function effectDud(colors) {
        spawnQueue.push({ at: 0, fn: () => launchRocket(colors, "fizzle", { x: width * rand(0.2, 0.32) }) });
        spawnQueue.push({ at: 500, fn: () => launchRocket(colors, "weak", { x: width * rand(0.45, 0.55) }) });
        spawnQueue.push({ at: 1000, fn: () => launchRocket(colors, "pop", { x: width * rand(0.68, 0.8) }) });
    }

    /** Score of 0: the fuse crackles, the rocket hops and gives up. */
    function effectMisfire(colors) {
        const x = width / 2;
        const base = height - 80;
        [0, 150, 300, 450, 600].forEach((at) => {
            spawnQueue.push({
                at,
                fn: () => glitter(x + rand(-3, 3), base, -Math.PI / 2 + rand(-1, 1), rand(20, 60),
                    "#e08a2c", rand(0.4, 0.6), { gravity: GRAVITY * 0.5, size: rand(2, 3.2) }),
            });
        });
        spawnQueue.push({ at: 800, fn: () => launchRocket(colors, "misfire", { x, y: base }) });
    }

    function effectFireworks(colors) {
        const volleys = [0, 350, 800, 1250, 1750, 2300, 2900];
        volleys.forEach((at) => {
            spawnQueue.push({ at, fn: () => launchRocket(colors) });
        });
        // A double rocket for good measure, then the finale.
        spawnQueue.push({ at: 1250, fn: () => launchRocket(colors) });
        spawnQueue.push({ at: 3300, fn: () => effectConfetti(colors) });
    }

    // ------------------------------------------------------------------
    // Animation loop
    // ------------------------------------------------------------------

    /** Draw a fading polyline through the last positions up to (x, y). */
    function drawTrail(hist, x, y, size, color, alpha) {
        if (!hist.length) return;
        ctx.strokeStyle = color;
        ctx.lineCap = "round";
        let prev = hist[0];
        for (let i = 1; i <= hist.length; i++) {
            const next = i < hist.length ? hist[i] : { x, y };
            ctx.globalAlpha = alpha * (i / (hist.length + 1)) * 0.7;
            ctx.lineWidth = Math.max(0.8, size * (i / (hist.length + 1)));
            ctx.beginPath();
            ctx.moveTo(prev.x, prev.y);
            ctx.lineTo(next.x, next.y);
            ctx.stroke();
            prev = next;
        }
        ctx.globalAlpha = 1;
    }

    function step(now) {
        const dt = Math.min((now - lastTime) / 1000, 0.05);
        lastTime = now;
        const elapsed = now - startedAt;

        // Timed spawns.
        spawnQueue = spawnQueue.filter((item) => {
            if (elapsed >= item.at) { item.fn(); return false; }
            return true;
        });

        // Clear fully each frame: only sparks and rockets draw trails
        // (from their own position history), so confetti stays crisp.
        ctx.clearRect(0, 0, width, height);

        // Rockets.
        rockets = rockets.filter((r) => {
            r.hist.push({ x: r.x, y: r.y });
            if (r.hist.length > TRAIL_LENGTH) r.hist.shift();
            r.x += r.vx * dt;
            r.y += r.vy * dt;
            // A tired rocket weaves left and right as it climbs.
            if (r.wobble) r.x += Math.sin(r.y * 0.09) * 1.4;
            r.vy += GRAVITY * 0.35 * dt;
            drawTrail(r.hist, r.x, r.y, 2.6, ROCKET_COLOR, 1);
            ctx.fillStyle = ROCKET_COLOR;
            ctx.beginPath();
            ctx.arc(r.x, r.y, 2.6, 0, Math.PI * 2);
            ctx.fill();
            if (r.y <= r.targetY || r.vy >= 0) {
                detonate(r);
                return false;
            }
            return true;
        });

        // Particles.
        particles = particles.filter((p) => {
            p.age += dt;
            if (p.age >= p.life) return false;
            if (p.trail) {
                p.hist.push({ x: p.x, y: p.y });
                if (p.hist.length > TRAIL_LENGTH) p.hist.shift();
            }
            const damping = Math.exp(-p.drag * dt);
            p.vx *= damping;
            p.vy = p.vy * damping + p.gravity * dt;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            if (p.y > height + 40 || p.x < -60 || p.x > width + 60) return false;

            if (p.kind === "smoke") {
                // Soft grey puff: swells while fading out.
                const s = p.age / p.life;
                ctx.globalAlpha = 0.34 * (1 - s);
                ctx.fillStyle = p.color;
                ctx.beginPath();
                ctx.arc(p.x, p.y, p.size * (1 + 2.4 * s), 0, Math.PI * 2);
                ctx.fill();
                return true;
            }

            const t = p.age / p.life;
            const fade = t < 0.65 ? 1 : 1 - (t - 0.65) / 0.35;
            ctx.globalAlpha = Math.max(0, fade);
            ctx.fillStyle = p.color;

            if (p.kind === "paper") {
                p.rot += p.vrot * dt;
                p.flip += p.vflip * dt;
                ctx.save();
                ctx.translate(p.x, p.y);
                ctx.rotate(p.rot);
                // |cos| fakes the paper flipping in 3D.
                ctx.scale(1, Math.max(0.15, Math.abs(Math.cos(p.flip))));
                ctx.fillRect(-p.w / 2, -p.h / 2, p.w, p.h);
                ctx.restore();
            } else {
                if (p.trail) drawTrail(p.hist, p.x, p.y, p.size, p.color, Math.max(0, fade));
                let size = p.size;
                if (p.twinkle) size *= 0.65 + 0.35 * Math.abs(Math.sin(p.age * p.twinkle));
                ctx.beginPath();
                ctx.arc(p.x, p.y, size, 0, Math.PI * 2);
                ctx.fill();
            }
            return true;
        });
        ctx.globalAlpha = 1;

        const busy = particles.length || rockets.length || spawnQueue.length;
        if (busy && elapsed < HARD_STOP_MS) {
            rafId = requestAnimationFrame(step);
        } else {
            stop();
        }
    }

    function stop() {
        if (rafId) cancelAnimationFrame(rafId);
        rafId = 0;
        clearTimeout(hardStopId);
        hardStopId = 0;
        particles = [];
        rockets = [];
        spawnQueue = [];
        window.removeEventListener("resize", resize);
        if (canvas) canvas.remove();
        canvas = null;
        ctx = null;
    }

    // ------------------------------------------------------------------
    // Public API
    // ------------------------------------------------------------------

    /** @returns {"none"|"misfire"|"dud"|"sparkle"|"confetti"|"fireworks"} */
    function celebrationLevel(correct, total) {
        if (!total) return "none";
        const ratio = correct / total;
        if (ratio >= 1 && total >= MIN_FOR_FIREWORKS) return "fireworks";
        if (ratio >= 0.8) return "confetti";
        if (ratio >= 0.5) return "sparkle";
        return correct === 0 ? "misfire" : "dud";
    }

    /**
     * @param {number} correct
     * @param {number} total
     * @param {Element} [anchor] Element the sparkle bursts from (the score).
     */
    function celebrate(correct, total, anchor) {
        stop();
        const level = celebrationLevel(correct, total);
        if (level === "none" || reducedMotion()) return;

        const colors = themeColors();
        ensureCanvas();

        let origin = { x: width / 2, y: height * 0.3 };
        if (anchor && anchor.getBoundingClientRect) {
            const rect = anchor.getBoundingClientRect();
            origin = { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
        }

        if (level === "sparkle") effectSparkle(origin, colors);
        else if (level === "confetti") effectConfetti(colors);
        else if (level === "dud") effectDud(colors);
        else if (level === "misfire") effectMisfire(colors);
        else effectFireworks(colors);

        startedAt = performance.now();
        lastTime = startedAt;
        rafId = requestAnimationFrame(step);
        hardStopId = setTimeout(stop, HARD_STOP_MS + 500);
    }

    // Leaving the page (or any route change) ends the party.
    window.addEventListener("hashchange", stop);

    window.celebrate = celebrate;
    window.stopCelebration = stop;
    window.celebrationLevel = celebrationLevel;
})();
