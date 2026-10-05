/**
 * geomap.js -- geographic map blocks ("% lat, lon | label" in SMD).
 *
 * Not to be confused with map.js (the adventure map of the homepage).
 *
 * renderBlock() in app.js creates the container and calls
 * Geomap.mount(container, block). Leaflet (vendored in
 * vendor/leaflet/, precached by the service worker) is loaded lazily,
 * only the first time a sheet with a map is opened, and the map itself
 * is only built once its container scrolls into view.
 *
 * Block shape (see smd2data.parse_geomap()):
 *   { type: "geomap", title, route, zoom, layer,
 *     points: [{ lat, lon, label }] }
 */
(function () {
    "use strict";

    const VENDOR = "vendor/leaflet/";

    // Base maps. Attribution is mandatory for every one of them: do not
    // remove it. Keep this list in sync with GEOMAP_LAYERS in smd2data.py
    // and with tools/geomap-editor.html.
    const OSM_ATTR = '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors';
    const CARTO_ATTR = OSM_ATTR + ' &copy; <a href="https://carto.com/attributions">CARTO</a>';
    const LAYERS = {
        osm: {
            url: "https://tile.openstreetmap.org/{z}/{x}/{y}.png",
            options: { maxZoom: 19, attribution: OSM_ATTR },
        },
        topo: {
            url: "https://{s}.tile.opentopomap.org/{z}/{x}/{y}.png",
            options: {
                maxZoom: 17,
                subdomains: "abc",
                attribution: 'Map data: ' + OSM_ATTR + ', <a href="https://viewfinderpanoramas.org">SRTM</a> | Map style: &copy; <a href="https://opentopomap.org">OpenTopoMap</a> (<a href="https://creativecommons.org/licenses/by-sa/3.0/">CC-BY-SA</a>)',
            },
        },
        light: {
            url: "https://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}{r}.png",
            options: { maxZoom: 20, subdomains: "abcd", attribution: CARTO_ATTR },
        },
        dark: {
            url: "https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png",
            options: { maxZoom: 20, subdomains: "abcd", attribution: CARTO_ATTR },
        },
    };

    let leafletPromise = null;

    function loadLeaflet() {
        if (window.L) return Promise.resolve(window.L);
        if (leafletPromise) return leafletPromise;
        leafletPromise = new Promise((resolve, reject) => {
            const css = document.createElement("link");
            css.rel = "stylesheet";
            css.href = VENDOR + "leaflet.css";
            document.head.appendChild(css);
            const script = document.createElement("script");
            script.src = VENDOR + "leaflet.js";
            script.onload = () => {
                window.L.Icon.Default.imagePath = VENDOR + "images/";
                resolve(window.L);
            };
            script.onerror = () => {
                leafletPromise = null;
                reject(new Error("Leaflet could not be loaded"));
            };
            document.head.appendChild(script);
        });
        return leafletPromise;
    }

    function formatCoords(point) {
        return point.lat.toFixed(4) + ", " + point.lon.toFixed(4);
    }

    /** Plain list of the points, used when Leaflet cannot load. */
    function renderFallback(container, block) {
        container.classList.add("geomap-fallback");
        container.textContent = "";
        const list = document.createElement("ol");
        block.points.forEach((point) => {
            const item = document.createElement("li");
            item.textContent = (point.label ? point.label + " — " : "") + formatCoords(point);
            list.appendChild(item);
        });
        container.appendChild(list);
    }

    function numberedIcon(L, number) {
        return L.divIcon({
            className: "geomap-step",
            html: "<span>" + number + "</span>",
            iconSize: [26, 26],
            iconAnchor: [13, 13],
            popupAnchor: [0, -12],
        });
    }

    function build(L, container, block) {
        const layer = LAYERS[block.layer] || LAYERS.osm;
        const map = L.map(container, { scrollWheelZoom: false });
        L.tileLayer(layer.url, layer.options).addTo(map);

        const latlngs = block.points.map((p) => [p.lat, p.lon]);
        block.points.forEach((point, i) => {
            const marker = L.marker(latlngs[i], block.route ? { icon: numberedIcon(L, i + 1) } : {});
            if (point.label) marker.bindPopup(point.label);
            marker.addTo(map);
        });
        if (block.route && latlngs.length > 1) {
            L.polyline(latlngs, { className: "geomap-route", weight: 4, opacity: 0.85 }).addTo(map);
        }

        if (latlngs.length === 1) {
            map.setView(latlngs[0], block.zoom || 13);
        } else {
            map.fitBounds(latlngs, { padding: [28, 28], maxZoom: block.zoom || 16 });
        }
    }

    /**
     * Fill `container` (already in the DOM or about to be) with the map
     * described by `block`. Safe to call once per container.
     */
    function mount(container, block) {
        container.classList.add("geomap");
        let started = false;
        const start = () => {
            if (started) return;
            started = true;
            loadLeaflet()
                .then((L) => build(L, container, block))
                .catch(() => renderFallback(container, block));
        };
        if ("IntersectionObserver" in window) {
            const observer = new IntersectionObserver((entries) => {
                if (entries.some((e) => e.isIntersecting)) {
                    observer.disconnect();
                    start();
                }
            }, { rootMargin: "200px" });
            observer.observe(container);
        } else {
            start();
        }
    }

    window.Geomap = { mount, LAYERS };
})();
