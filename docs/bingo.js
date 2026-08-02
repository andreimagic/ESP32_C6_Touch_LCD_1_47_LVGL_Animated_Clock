// Bingo ticket generator — port of the ESP32's own /bingo web route
// (BINGO_PAGE in the .ino). Kept functionally identical so cards printed
// from either source follow the same UK/housie layout rules; mirror any
// changes to the generation algorithm in both places.
document.addEventListener('DOMContentLoaded', () => {
    const sheetEl = document.getElementById('bgc-sheet');
    if (!sheetEl) return; // only run on the Bingo Cards page

    let ticketsPerPage = 6;

    // Fixed known-valid mask: rows 5/5/5, every column 1-3 numbers, total 15.
    // Only ever used if the random generator somehow fails to converge.
    const FALLBACK = [
        [1, 0, 1, 1, 0, 1, 1, 0, 0],
        [1, 1, 0, 1, 1, 0, 0, 1, 0],
        [0, 1, 1, 0, 1, 1, 0, 0, 1]
    ];

    // Column counts: all start at 1 (=9), distribute the remaining 6, cap 3.
    function counts() {
        const c = [1, 1, 1, 1, 1, 1, 1, 1, 1];
        let extra = 6;
        while (extra > 0) {
            const i = Math.floor(Math.random() * 9);
            if (c[i] < 3) { c[i]++; extra--; }
        }
        return c;
    }

    // Place each column's numbers into rows, filling the emptiest rows first
    // so all three rows land on exactly 5. Returns null if it paints itself in.
    function mask(c) {
        const m = [[0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0]];
        const left = [5, 5, 5];
        const cols = [0, 1, 2, 3, 4, 5, 6, 7, 8];
        cols.sort((a, b) => c[b] - c[a]);
        for (let i = 0; i < 9; i++) {
            const col = cols[i], n = c[col];
            const rows = [0, 1, 2];
            rows.sort((a, b) => {
                if (left[b] !== left[a]) return left[b] - left[a];
                return Math.random() - 0.5;
            });
            for (let k = 0; k < n; k++) {
                const r = rows[k];
                if (left[r] <= 0) return null;
                m[r][col] = 1;
                left[r]--;
            }
        }
        if (left[0] || left[1] || left[2]) return null;
        return m;
    }

    // Build one ticket: pick the layout, then draw each column's numbers from
    // its own decade and sort them top-to-bottom (as on a real ticket).
    function ticket() {
        let m = null;
        for (let a = 0; a < 200 && !m; a++) m = mask(counts());
        if (!m) m = FALLBACK;
        const g = [[], [], []];
        for (let col = 0; col < 9; col++) {
            const lo = (col === 0) ? 1 : col * 10;
            const hi = (col === 8) ? 90 : col * 10 + 9;
            const pool = [];
            for (let v = lo; v <= hi; v++) pool.push(v);
            for (let i = pool.length - 1; i > 0; i--) {
                const j = Math.floor(Math.random() * (i + 1));
                const t = pool[i]; pool[i] = pool[j]; pool[j] = t;
            }
            let need = 0;
            for (let r = 0; r < 3; r++) if (m[r][col]) need++;
            const pick = pool.slice(0, need).sort((a, b) => a - b);
            let k = 0;
            for (let r = 0; r < 3; r++) g[r][col] = m[r][col] ? pick[k++] : 0;
        }
        return g;
    }

    function build() {
        let out = '';
        for (let t = 0; t < ticketsPerPage; t++) {
            const g = ticket();
            out += `<div class="bgc-ticket"><div class="bgc-lbl">TICKET ${t + 1}</div><table>`;
            for (let r = 0; r < 3; r++) {
                out += '<tr>';
                for (let c = 0; c < 9; c++) out += `<td>${g[r][c] ? g[r][c] : ''}</td>`;
                out += '</tr>';
            }
            out += '</table></div>';
        }
        sheetEl.innerHTML = out;
    }

    function setN(n) {
        ticketsPerPage = n;
        sheetEl.className = 'bgc-sheet bgc-d' + n;
        [4, 6, 8].forEach(v => {
            const btn = document.getElementById('bgc-n' + v);
            if (btn) btn.classList.toggle('on', v === n);
        });
        build();
    }

    [4, 6, 8].forEach(n => {
        const btn = document.getElementById('bgc-n' + n);
        if (btn) btn.addEventListener('click', () => setN(n));
    });

    const newBtn = document.getElementById('bgc-new');
    if (newBtn) newBtn.addEventListener('click', build);

    const printBtn = document.getElementById('bgc-print');
    if (printBtn) printBtn.addEventListener('click', () => window.print());

    build();
});
