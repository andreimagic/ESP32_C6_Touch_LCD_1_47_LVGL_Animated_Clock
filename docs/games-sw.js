/* Offline support for the browser games.
   Caches the game pages and their icons so the home-screen app opens in airplane mode.
   Game files are network-first, so updates you publish arrive as soon as the device is
   online; when it is offline (or the network is slow) the cached copy is used.
   Everything else on the site (firmware flasher, docs, ...) is left alone.
   Settings and high scores live in localStorage and are not touched by this file. */
const CACHE = 'letters-games-v1';
const FILES = [
  'letters-rain.html', 'snake-letters.html', 'tennis-letters.html',
  'games.webmanifest',
  'assets/icon-192.png', 'assets/icon-512.png',
  'assets/favicon-16.png', 'assets/favicon-32.png',
  'assets/apple-touch-icon-letters.png'
];
const BASE = new URL('./', self.location).href;
const URLS = FILES.map(f => new URL(f, BASE).href);
const OURS = new Set(URLS);

self.addEventListener('install', e => {
  e.waitUntil((async () => {
    const cache = await caches.open(CACHE);
    // one by one, so a single missing file cannot block the install
    await Promise.allSettled(URLS.map(u => cache.add(new Request(u, { cache: 'reload' }))));
    await self.skipWaiting();
  })());
});

self.addEventListener('activate', e => {
  e.waitUntil((async () => {
    const keys = await caches.keys();
    await Promise.all(keys.filter(k => k !== CACHE && k.startsWith('letters-games-')).map(k => caches.delete(k)));
    await self.clients.claim();
  })());
});

self.addEventListener('fetch', e => {
  const req = e.request;
  if (req.method !== 'GET') return;
  const url = new URL(req.url);
  url.search = ''; url.hash = '';
  if (!OURS.has(url.href)) return;          // not a game file: let the browser handle it
  e.respondWith(networkFirst(req, url.href));
});

function withTimeout(p, ms) {
  return new Promise((ok, no) => {
    const t = setTimeout(() => no(new Error('timeout')), ms);
    p.then(v => { clearTimeout(t); ok(v); }, err => { clearTimeout(t); no(err); });
  });
}

async function networkFirst(req, key) {
  const cache = await caches.open(CACHE);
  const cached = await cache.match(key);
  try {
    const res = await (cached ? withTimeout(fetch(req), 4000) : fetch(req));
    if (res && res.ok) { cache.put(key, res.clone()); return res; }
    return cached || res;
  } catch (err) {
    if (cached) return cached;
    throw err;
  }
}
