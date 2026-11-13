/* ============================================================================
 *  web/app.js  ——  前端逻辑
 *
 *  它只做三件事：
 *    ① 向 C 程序要数据   fetch('/api/state')
 *    ② 把数据画成格子
 *    ③ 点按钮时告诉 C 程序  fetch('/api/water?x=1&y=2')
 *
 *  它不算任何算法 —— 排序、查找、队列全在 C 那边。
 * ========================================================================== */

const $ = (id) => document.getElementById(id);

let state = null;      // 最近一次拿到的数据
let cursor = { x: 0, y: 0 };   // 当前选中的格子
let selCrop = 0;       // 当前选的种子 id
let busy = false;

/* 格子里画什么：用作物名的第一个字。
 *
 * 为什么不放小图标（🌾🍅🌽）？因为那些是 emoji，要靠系统字体支持。
 * 机房的旧电脑、Win7、部分 Linux 上会显示成一个空方块。
 * 用汉字最稳 —— 任何一台能显示中文的电脑都能正常显示。 */
function glyph(name) {
  return name ? name.charAt(0) : '';
}

/* ---------------------------------------------------------------- 提示 */
let toastTimer = 0;
function toast(msg) {
  const el = $('toast');
  el.textContent = msg;
  el.classList.add('on');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.remove('on'), 2200);
}

/* ---------------------------------------------------------------- 取数据 */
async function load() {
  if (busy) return;
  try {
    const res = await fetch('/api/state', { cache: 'no-store' });
    if (!res.ok) throw new Error('HTTP ' + res.status);
    state = await res.json();
    render();
  } catch (e) {
    $('cursorInfo').innerHTML =
      '<b style="color:#b91c1c">连不上 C 程序</b>　' +
      '确认 <code>pf serve</code> 还开着，然后刷新页面';
  }
}

/* ---------------------------------------------------------------- 画 */
function render() {
  if (!state) return;

  $('day').textContent = state.day;
  $('clock').textContent =
    String(state.hour).padStart(2, '0') + ':' + String(state.minute).padStart(2, '0');
  $('gold').textContent = state.gold;
  $('stamina').textContent = state.stamina;
  $('cmp').textContent = state.stats.cmp;
  $('mov').textContent = state.stats.mov;

  const cm = state.stats.cost_min;
  $('cost').textContent = cm < 60 ? cm + ' 分'
                        : Math.floor(cm / 60) + ' 时 ' + (cm % 60) + ' 分';

  /* ---- 格子 ---- */
  const grid = $('grid');
  const need = state.plots.length;

  if (grid.children.length !== need) {
    grid.innerHTML = '';
    for (let i = 0; i < need; i++) {
      const d = document.createElement('div');
      d.className = 'plot';
      d.dataset.i = i;
      d.innerHTML =
        '<span class="icon"></span>' +
        '<span class="name"></span>' +
        '<span class="pct"></span>' +
        '<span class="bar" style="width:0"></span>';
      d.onclick = () => { cursor = { x: +d.dataset.x, y: +d.dataset.y }; render(); };
      grid.appendChild(d);
    }
  }

  state.plots.forEach((p, i) => {
    const d = grid.children[i];
    d.dataset.x = p.x;
    d.dataset.y = p.y;
    d.className = 'plot s-' + p.state +
      (p.x === cursor.x && p.y === cursor.y ? ' sel' : '');

    const icon = d.querySelector('.icon');
    const name = d.querySelector('.name');
    const pct  = d.querySelector('.pct');
    const bar  = d.querySelector('.bar');

    if (p.state === 'wild') {
      icon.textContent = '·';   name.textContent = '';  pct.textContent = ''; bar.style.width = '0';
    } else if (p.state === 'tilled') {
      icon.textContent = '';    name.textContent = '';  pct.textContent = ''; bar.style.width = '0';
    } else if (p.state === 'dead') {
      icon.textContent = '×';   name.textContent = '';  pct.textContent = ''; bar.style.width = '0';
    } else {
      icon.textContent = glyph(p.crop);
      name.textContent = p.state === 'ripe' ? '可收' : p.crop;
      pct.textContent = p.state === 'ripe' ? '' : p.growth + '%';
      bar.style.width = p.growth + '%';
    }
  });

  /* ---- 光标信息 ---- */
  const cur = state.plots.find(p => p.x === cursor.x && p.y === cursor.y);
  if (cur) {
    const map = {
      wild: '荒地。按「开垦」',
      tilled: '已开垦，可以播种',
      seeded: '已播种，等发芽',
      growing: '生长中 ' + cur.growth + '%，水分 ' + cur.water,
      ripe: '<b style="color:#b45309">熟了！</b>按「收获」',
      dead: '枯萎了，重新开垦吧'
    };
    $('cursorInfo').innerHTML =
      '第 <b>(' + cur.x + ',' + cur.y + ')</b> 块　' + (map[cur.state] || '');
  }

  /* ---- 仓库 ---- */
  const barn = $('barn');
  if (!state.barn.length) {
    barn.innerHTML = '<p class="empty">空的</p>';
  } else {
    barn.innerHTML = state.barn.map(b =>
      '<div class="row"><span><i class="dot">' + glyph(b.crop) + '</i>' + b.crop +
      '</span><b>' + b.count + ' 斤</b></div>').join('');
  }
  $('barnCount').textContent = state.barn.length + ' 种';

  /* ---- 行情 ---- */
  const mk = $('market');
  mk.innerHTML = state.crops.map(c =>
    '<div class="row' + (c.id === selCrop ? ' on' : '') + '" data-id="' + c.id + '">' +
      '<span class="nm"><i class="dot">' + glyph(c.name) + '</i>' + c.name + '</span>' +
      '<span class="d">' + c.days + ' 天</span>' +
      '<span class="pr">' + c.price + ' 分</span>' +
    '</div>').join('');

  mk.querySelectorAll('.row').forEach(row => {
    row.onclick = () => {
      selCrop = +row.dataset.id;
      mk.querySelectorAll('.row').forEach(r => r.classList.remove('on'));
      row.classList.add('on');
    };
  });
}

/* ---------------------------------------------------------------- 动作 */
async function act(name) {
  if (busy) return;
  busy = true;

  try {
    let url = '/api/' + name;
    if (name !== 'nextday' && name !== 'order' && name !== 'sell') {
      url += '?x=' + cursor.x + '&y=' + cursor.y;
    }
    const res = await fetch(url, { cache: 'no-store' });
    const r = await res.json();
    if (r.toast) toast(r.toast);
    await load();
  } catch (e) {
    toast('操作失败：' + e.message);
  } finally {
    busy = false;
  }
}

/* 播种要先选种子：直接用当前行情里选中的那个 */
async function sow() {
  if (busy) return;
  busy = true;
  try {
    const res = await fetch('/api/sow?x=' + cursor.x + '&y=' + cursor.y +
                            '&c=' + selCrop, { cache: 'no-store' });
    const r = await res.json();
    if (r.toast) toast(r.toast);
    await load();
  } finally {
    busy = false;
  }
}

/* ---------------------------------------------------------------- 绑定 */
document.querySelectorAll('.btns button').forEach(btn => {
  btn.onclick = () => {
    const a = btn.dataset.act;
    if (a === 'sow') sow();
    else act(a);
  };
});

/* 键盘快捷键，和终端版一致 */
document.addEventListener('keydown', (e) => {
  const k = e.key.toLowerCase();
  const map = { t: 'till', w: 'water', h: 'harvest', o: 'order', v: 'sell', n: 'nextday' };
  if (k === 's') { sow(); return; }
  if (map[k]) { act(map[k]); return; }
  if (e.key === 'ArrowUp')    { cursor.y = Math.max(0, cursor.y - 1); render(); }
  if (e.key === 'ArrowDown')  { cursor.y = Math.min(5, cursor.y + 1); render(); }
  if (e.key === 'ArrowLeft')  { cursor.x = Math.max(0, cursor.x - 1); render(); }
  if (e.key === 'ArrowRight') { cursor.x = Math.min(5, cursor.x + 1); render(); }
});

/* 启动 */
load();
setInterval(load, 1500);
