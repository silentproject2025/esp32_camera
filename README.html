<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>SANZXCAM v5.9-fix — README</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@300;400;500;700&family=Share+Tech+Mono&display=swap');

  :root {
    --black: #080808;
    --gray-1: #111;
    --gray-2: #1a1a1a;
    --gray-3: #222;
    --gray-4: #2e2e2e;
    --gray-5: #3a3a3a;
    --gray-6: #555;
    --gray-7: #777;
    --gray-8: #999;
    --gray-9: #bbb;
    --gray-a: #ccc;
    --gray-b: #ddd;
    --white: #f0f0f0;
    --accent: #e8e8e8;
    --dim: #444;
    --border: #2a2a2a;
    --highlight: #1e1e1e;
    --green: #4a8;
    --green-dim: #264;
    --red: #a44;
    --amber: #a83;
    --blue: #48a;
  }

  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  html { scroll-behavior: smooth; }

  body {
    background: var(--black);
    color: var(--gray-9);
    font-family: 'JetBrains Mono', 'Share Tech Mono', monospace;
    font-size: 13px;
    line-height: 1.7;
    overflow-x: hidden;
  }

  /* SCANLINE OVERLAY */
  body::before {
    content: '';
    position: fixed;
    inset: 0;
    background: repeating-linear-gradient(0deg, transparent, transparent 2px, rgba(0,0,0,0.03) 2px, rgba(0,0,0,0.03) 4px);
    pointer-events: none;
    z-index: 9999;
  }

  /* HEADER */
  .site-header {
    border-bottom: 1px solid var(--border);
    padding: 0 40px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    height: 44px;
    background: var(--gray-1);
    position: sticky;
    top: 0;
    z-index: 100;
  }
  .site-header .logo {
    font-size: 11px;
    letter-spacing: 0.15em;
    color: var(--gray-6);
    text-transform: uppercase;
  }
  .site-header .logo span { color: var(--gray-9); }
  .header-nav { display: flex; gap: 28px; }
  .header-nav a {
    color: var(--gray-6);
    text-decoration: none;
    font-size: 10px;
    letter-spacing: 0.12em;
    text-transform: uppercase;
    transition: color 0.15s;
  }
  .header-nav a:hover { color: var(--white); }

  /* HERO */
  .hero {
    border-bottom: 1px solid var(--border);
    padding: 64px 40px 56px;
    position: relative;
    overflow: hidden;
  }
  .hero::after {
    content: '';
    position: absolute;
    right: -60px;
    top: -40px;
    width: 400px;
    height: 400px;
    background: radial-gradient(circle, rgba(60,60,60,0.15) 0%, transparent 70%);
    pointer-events: none;
  }

  .ascii-logo {
    font-size: 10px;
    line-height: 1.2;
    color: var(--gray-5);
    letter-spacing: 0;
    white-space: pre;
    margin-bottom: 28px;
    font-family: 'Share Tech Mono', monospace;
    animation: fadeIn 0.6s ease;
  }
  .ascii-logo span { color: var(--gray-7); }

  @keyframes fadeIn { from { opacity: 0; transform: translateY(-8px); } to { opacity: 1; transform: none; } }
  @keyframes slideIn { from { opacity: 0; transform: translateX(-12px); } to { opacity: 1; transform: none; } }
  @keyframes blinkCursor { 0%, 100% { opacity: 1; } 50% { opacity: 0; } }

  .hero-title {
    font-size: 28px;
    font-weight: 700;
    color: var(--white);
    letter-spacing: -0.01em;
    margin-bottom: 8px;
    animation: slideIn 0.5s ease 0.1s both;
  }
  .hero-title .ver {
    font-size: 14px;
    color: var(--gray-6);
    font-weight: 300;
    margin-left: 12px;
    letter-spacing: 0.05em;
  }
  .hero-sub {
    color: var(--gray-7);
    font-size: 12px;
    letter-spacing: 0.04em;
    margin-bottom: 32px;
    animation: slideIn 0.5s ease 0.2s both;
  }
  .hero-sub::before { content: '// '; color: var(--gray-5); }

  .badge-row {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    animation: slideIn 0.5s ease 0.3s both;
  }
  .badge {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    background: var(--gray-2);
    border: 1px solid var(--border);
    border-radius: 3px;
    padding: 4px 10px;
    font-size: 10px;
    letter-spacing: 0.08em;
    color: var(--gray-8);
  }
  .badge .dot {
    width: 5px; height: 5px; border-radius: 50%;
    background: var(--gray-6);
    flex-shrink: 0;
  }
  .badge.green .dot { background: var(--green); }
  .badge.amber .dot { background: var(--amber); }
  .badge.red   .dot { background: var(--red); }
  .badge.blue  .dot { background: var(--blue); }

  /* LAYOUT */
  .layout {
    display: grid;
    grid-template-columns: 220px 1fr;
    min-height: calc(100vh - 44px);
  }

  /* SIDEBAR */
  .sidebar {
    border-right: 1px solid var(--border);
    padding: 28px 0;
    position: sticky;
    top: 44px;
    height: calc(100vh - 44px);
    overflow-y: auto;
    background: var(--gray-1);
  }
  .sidebar::-webkit-scrollbar { width: 3px; }
  .sidebar::-webkit-scrollbar-track { background: transparent; }
  .sidebar::-webkit-scrollbar-thumb { background: var(--gray-4); border-radius: 2px; }

  .sidebar-section { margin-bottom: 24px; }
  .sidebar-label {
    font-size: 9px;
    letter-spacing: 0.18em;
    text-transform: uppercase;
    color: var(--gray-5);
    padding: 0 20px;
    margin-bottom: 6px;
    display: block;
  }
  .sidebar a {
    display: block;
    padding: 5px 20px;
    color: var(--gray-7);
    text-decoration: none;
    font-size: 11px;
    border-left: 2px solid transparent;
    transition: all 0.12s;
  }
  .sidebar a:hover {
    color: var(--gray-a);
    border-left-color: var(--gray-5);
    background: var(--gray-2);
  }
  .sidebar a.active {
    color: var(--white);
    border-left-color: var(--white);
    background: var(--gray-2);
  }

  /* CONTENT */
  .content {
    padding: 48px 56px;
    max-width: 900px;
  }

  /* SECTIONS */
  .section {
    margin-bottom: 72px;
    animation: fadeIn 0.4s ease;
  }

  .section-header {
    display: flex;
    align-items: baseline;
    gap: 14px;
    margin-bottom: 28px;
    padding-bottom: 12px;
    border-bottom: 1px solid var(--border);
  }
  .section-num {
    font-size: 10px;
    color: var(--gray-5);
    letter-spacing: 0.1em;
    font-weight: 300;
  }
  .section-title {
    font-size: 15px;
    font-weight: 700;
    color: var(--white);
    letter-spacing: 0.04em;
    text-transform: uppercase;
  }

  h3 {
    font-size: 12px;
    font-weight: 500;
    color: var(--gray-a);
    letter-spacing: 0.08em;
    text-transform: uppercase;
    margin: 28px 0 12px;
    display: flex;
    align-items: center;
    gap: 8px;
  }
  h3::before { content: '▸'; color: var(--gray-5); font-size: 10px; }

  p {
    color: var(--gray-8);
    margin-bottom: 14px;
    font-size: 12px;
    line-height: 1.8;
  }

  /* CODE BLOCKS */
  pre, code {
    font-family: 'JetBrains Mono', monospace;
  }
  code {
    background: var(--gray-2);
    border: 1px solid var(--gray-3);
    border-radius: 2px;
    padding: 1px 5px;
    font-size: 11px;
    color: var(--gray-a);
  }
  pre {
    background: var(--gray-1);
    border: 1px solid var(--border);
    border-radius: 4px;
    padding: 18px 20px;
    overflow-x: auto;
    margin: 14px 0 20px;
    font-size: 11px;
    line-height: 1.65;
    color: var(--gray-8);
    position: relative;
  }
  pre .ln { color: var(--gray-4); margin-right: 16px; user-select: none; font-size: 10px; }
  pre .cm { color: var(--gray-5); }
  pre .kw { color: var(--gray-9); font-weight: 500; }
  pre .st { color: var(--gray-7); }
  pre .fn { color: var(--gray-a); }
  pre .ok { color: var(--green); }
  pre .er { color: var(--red); }
  pre .am { color: var(--amber); }
  pre .bl { color: var(--blue); }
  pre .hi { color: var(--white); font-weight: 500; }
  pre .prompt { color: var(--gray-5); }

  /* TABLES */
  table {
    width: 100%;
    border-collapse: collapse;
    margin: 14px 0 24px;
    font-size: 11px;
  }
  thead tr {
    border-bottom: 1px solid var(--gray-4);
  }
  th {
    text-align: left;
    padding: 8px 12px;
    color: var(--gray-6);
    font-weight: 400;
    font-size: 9px;
    letter-spacing: 0.14em;
    text-transform: uppercase;
  }
  td {
    padding: 8px 12px;
    color: var(--gray-8);
    border-bottom: 1px solid var(--gray-2);
    vertical-align: top;
  }
  tr:hover td { background: var(--gray-1); }
  td code { background: none; border: none; padding: 0; color: var(--gray-a); }
  td .tag {
    display: inline-block;
    background: var(--gray-2);
    border: 1px solid var(--gray-3);
    border-radius: 2px;
    padding: 1px 6px;
    font-size: 9px;
    letter-spacing: 0.06em;
    color: var(--gray-7);
  }
  td .tag.ok  { color: var(--green); border-color: var(--green-dim); background: rgba(68,170,136,0.06); }
  td .tag.warn { color: var(--amber); border-color: #542; background: rgba(170,136,68,0.06); }
  td .tag.err  { color: var(--red);   border-color: #522; background: rgba(170,68,68,0.06); }
  td .tag.info { color: var(--blue);  border-color: #246; background: rgba(68,136,170,0.06); }

  /* BUTTON MAP */
  .btn-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
    margin: 16px 0 24px;
  }
  .btn-card {
    background: var(--gray-1);
    border: 1px solid var(--border);
    border-radius: 4px;
    overflow: hidden;
  }
  .btn-card-header {
    background: var(--gray-2);
    border-bottom: 1px solid var(--border);
    padding: 8px 14px;
    display: flex;
    align-items: center;
    gap: 10px;
  }
  .btn-chip {
    background: var(--gray-3);
    border: 1px solid var(--gray-4);
    border-radius: 3px;
    padding: 3px 8px;
    font-size: 10px;
    font-weight: 700;
    color: var(--white);
    letter-spacing: 0.06em;
  }
  .btn-chip.boot { border-color: var(--gray-6); }
  .btn-chip.b    { border-color: var(--gray-5); }
  .btn-chip.c    { border-color: var(--gray-5); }
  .btn-chip.d    { border-color: var(--gray-5); }
  .btn-label {
    font-size: 9px;
    color: var(--gray-5);
    letter-spacing: 0.1em;
    text-transform: uppercase;
  }
  .btn-card-body { padding: 10px 14px; }
  .btn-action {
    display: flex;
    align-items: flex-start;
    gap: 8px;
    padding: 5px 0;
    border-bottom: 1px solid var(--gray-2);
    font-size: 11px;
  }
  .btn-action:last-child { border-bottom: none; }
  .btn-press {
    flex-shrink: 0;
    font-size: 9px;
    color: var(--gray-5);
    padding-top: 1px;
    min-width: 38px;
    letter-spacing: 0.06em;
  }
  .btn-desc { color: var(--gray-8); line-height: 1.5; }

  /* MODE DIAGRAM */
  .mode-flow {
    display: flex;
    flex-direction: column;
    gap: 0;
    margin: 16px 0 24px;
    border: 1px solid var(--border);
    border-radius: 4px;
    overflow: hidden;
  }
  .mode-row {
    display: grid;
    grid-template-columns: 140px 1fr;
    border-bottom: 1px solid var(--border);
  }
  .mode-row:last-child { border-bottom: none; }
  .mode-name {
    background: var(--gray-2);
    padding: 10px 14px;
    font-size: 10px;
    font-weight: 700;
    color: var(--gray-a);
    letter-spacing: 0.08em;
    display: flex;
    align-items: center;
    border-right: 1px solid var(--border);
  }
  .mode-keys {
    padding: 10px 14px;
    font-size: 11px;
    color: var(--gray-7);
    display: flex;
    flex-direction: column;
    gap: 3px;
  }
  .mode-key-row { display: flex; align-items: baseline; gap: 8px; }
  .key-combo {
    flex-shrink: 0;
    min-width: 80px;
    font-size: 9px;
    color: var(--gray-5);
    letter-spacing: 0.05em;
  }
  .key-action { color: var(--gray-8); font-size: 11px; }

  /* FEATURE CARDS */
  .feature-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 10px;
    margin: 16px 0 24px;
  }
  .feature-card {
    background: var(--gray-1);
    border: 1px solid var(--border);
    border-radius: 4px;
    padding: 14px 16px;
    position: relative;
    overflow: hidden;
  }
  .feature-card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0;
    height: 2px;
    background: var(--gray-3);
  }
  .feature-card.stego::before { background: linear-gradient(90deg, #2a4a3a, #1a3a2a); }
  .feature-card.flash::before { background: linear-gradient(90deg, #3a3a1a, #2a2a0a); }
  .feature-card.face::before  { background: linear-gradient(90deg, #1a2a3a, #0a1a2a); }
  .feature-card.rec::before   { background: linear-gradient(90deg, #3a1a1a, #2a0a0a); }
  .fc-icon {
    font-size: 9px;
    letter-spacing: 0.12em;
    text-transform: uppercase;
    color: var(--gray-5);
    margin-bottom: 6px;
  }
  .fc-title {
    font-size: 12px;
    font-weight: 700;
    color: var(--gray-a);
    letter-spacing: 0.04em;
    margin-bottom: 6px;
  }
  .fc-desc {
    font-size: 10px;
    color: var(--gray-6);
    line-height: 1.6;
  }

  /* TERMINAL BLOCKS */
  .terminal {
    background: var(--gray-1);
    border: 1px solid var(--border);
    border-radius: 4px;
    overflow: hidden;
    margin: 14px 0 20px;
    font-size: 11px;
  }
  .terminal-header {
    background: var(--gray-2);
    border-bottom: 1px solid var(--border);
    padding: 7px 14px;
    display: flex;
    align-items: center;
    gap: 10px;
  }
  .term-dots { display: flex; gap: 5px; }
  .term-dot {
    width: 8px; height: 8px; border-radius: 50%;
    background: var(--gray-4);
  }
  .term-title {
    font-size: 9px;
    color: var(--gray-5);
    letter-spacing: 0.12em;
    text-transform: uppercase;
  }
  .terminal-body { padding: 16px 18px; line-height: 1.7; }
  .t-section { color: var(--gray-6); font-size: 10px; letter-spacing: 0.1em; margin: 10px 0 4px; }
  .t-row { display: flex; gap: 6px; }
  .t-key { color: var(--gray-5); min-width: 90px; flex-shrink: 0; }
  .t-dot { color: var(--gray-4); flex: 1; overflow: hidden; }
  .t-val-ok   { color: var(--green); }
  .t-val-dim  { color: var(--gray-6); }
  .t-val-hi   { color: var(--gray-a); }
  .t-val-warn { color: var(--amber); }
  .t-ready { text-align: center; color: var(--gray-8); letter-spacing: 0.3em; margin-top: 10px; font-size: 10px; }
  .t-bar { color: var(--gray-6); letter-spacing: 0; }

  /* PINOUT */
  .pin-table-wrap {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
    margin: 16px 0 24px;
  }
  .pin-group {
    background: var(--gray-1);
    border: 1px solid var(--border);
    border-radius: 4px;
    overflow: hidden;
  }
  .pin-group-header {
    background: var(--gray-2);
    border-bottom: 1px solid var(--border);
    padding: 7px 12px;
    font-size: 9px;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: var(--gray-6);
  }
  .pin-row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 5px 12px;
    border-bottom: 1px solid var(--gray-2);
    font-size: 11px;
  }
  .pin-row:last-child { border-bottom: none; }
  .pin-sig { color: var(--gray-7); }
  .pin-num {
    background: var(--gray-3);
    border: 1px solid var(--gray-4);
    border-radius: 2px;
    padding: 1px 7px;
    font-size: 10px;
    color: var(--gray-a);
    font-weight: 500;
  }

  /* CHANGELOG */
  .cl-entry {
    margin-bottom: 24px;
    padding-left: 20px;
    border-left: 2px solid var(--gray-3);
    position: relative;
  }
  .cl-entry::before {
    content: '';
    position: absolute;
    left: -5px; top: 7px;
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--gray-4);
    border: 2px solid var(--gray-2);
  }
  .cl-entry.fix::before { background: var(--green); }
  .cl-entry.new::before { background: var(--blue); }
  .cl-tag {
    display: inline-block;
    font-size: 9px;
    letter-spacing: 0.1em;
    padding: 2px 7px;
    border-radius: 2px;
    margin-bottom: 6px;
    text-transform: uppercase;
  }
  .cl-tag.fix  { background: rgba(68,170,136,0.1); color: var(--green); border: 1px solid rgba(68,170,136,0.2); }
  .cl-tag.new  { background: rgba(68,136,170,0.1); color: var(--blue);  border: 1px solid rgba(68,136,170,0.2); }
  .cl-title { font-size: 13px; font-weight: 700; color: var(--gray-a); margin-bottom: 6px; }
  .cl-body { font-size: 11px; color: var(--gray-7); line-height: 1.75; }
  .cl-body ul { padding-left: 16px; margin-top: 6px; }
  .cl-body li { margin-bottom: 4px; }

  /* FOOTER */
  footer {
    border-top: 1px solid var(--border);
    padding: 24px 40px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 10px;
    color: var(--gray-5);
    background: var(--gray-1);
  }
  footer span { color: var(--gray-7); }

  /* CURSOR BLINK */
  .cursor {
    display: inline-block;
    width: 7px; height: 12px;
    background: var(--gray-6);
    margin-left: 2px;
    vertical-align: middle;
    animation: blinkCursor 1s step-end infinite;
  }

  /* MISC */
  .divider {
    border: none;
    border-top: 1px solid var(--border);
    margin: 28px 0;
  }
  .tip {
    background: var(--gray-1);
    border: 1px solid var(--border);
    border-left: 3px solid var(--gray-5);
    border-radius: 0 4px 4px 0;
    padding: 10px 14px;
    font-size: 11px;
    color: var(--gray-7);
    margin: 12px 0 20px;
    line-height: 1.65;
  }
  .tip strong { color: var(--gray-9); font-weight: 500; }
  .tip.warn { border-left-color: var(--amber); }
  .tip.ok   { border-left-color: var(--green); }
  .tip.info { border-left-color: var(--blue); }

  a { color: var(--gray-7); text-decoration: none; }
  a:hover { color: var(--white); }

  @media (max-width: 800px) {
    .layout { grid-template-columns: 1fr; }
    .sidebar { display: none; }
    .content { padding: 28px 20px; }
    .btn-grid, .feature-grid, .pin-table-wrap { grid-template-columns: 1fr; }
  }
</style>
</head>
<body>

<!-- HEADER -->
<header class="site-header">
  <div class="logo"><span>SANZXCAM</span> / README / v5.9-fix</div>
  <nav class="header-nav">
    <a href="#overview">Overview</a>
    <a href="#hardware">Hardware</a>
    <a href="#buttons">Tombol</a>
    <a href="#features">Fitur</a>
    <a href="#stego">Stego</a>
    <a href="#changelog">Changelog</a>
  </nav>
</header>

<main>
<!-- HERO -->
<div class="hero">
  <pre class="ascii-logo"><span>███████╗ █████╗ ███╗   ██╗███████╗██╗  ██╗ ██████╗ █████╗ ███╗   ███╗</span>
<span>██╔════╝██╔══██╗████╗  ██║╚════██║╚██╗██╔╝██╔════╝██╔══██╗████╗ ████║</span>
███████╗███████║██╔██╗ ██║    ██╔╝ ╚███╔╝ ██║     ███████║██╔████╔██║
<span>╚════██║██╔══██║██║╚██╗██║   ██╔╝  ██╔██╗ ██║     ██╔══██║██║╚██╔╝██║</span>
███████║██║  ██║██║ ╚████║   ██║  ██╔╝ ██╗╚██████╗██║  ██║██║ ╚═╝ ██║
<span>╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝  ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚═╝</span></pre>

  <div class="hero-title">SANZXCAM<span class="ver">v5.9-fix</span><span class="cursor"></span></div>
  <div class="hero-sub">ESP32-S3 Camera System — Monochrome Terminal Aesthetic</div>

  <div class="badge-row">
    <div class="badge green"><div class="dot"></div>ESP32-S3-WROOM</div>
    <div class="badge"><div class="dot"></div>ILI9341 320×240</div>
    <div class="badge amber"><div class="dot"></div>GC2145 / OV3660</div>
    <div class="badge blue"><div class="dot"></div>BMP + JPG + MJPEG</div>
    <div class="badge"><div class="dot"></div>Steganografi LSB/COM</div>
    <div class="badge"><div class="dot"></div>EXIF Inject</div>
    <div class="badge red"><div class="dot"></div>Face Detect MSR01+MNP01</div>
    <div class="badge"><div class="dot"></div>USB Mass Storage</div>
  </div>
</div>

<!-- LAYOUT -->
<div class="layout">

<!-- SIDEBAR -->
<aside class="sidebar">
  <div class="sidebar-section">
    <span class="sidebar-label">Mulai</span>
    <a href="#overview">Overview</a>
    <a href="#hardware">Hardware</a>
    <a href="#pinout">Pinout</a>
    <a href="#install">Instalasi</a>
    <a href="#boot">Boot Sequence</a>
  </div>
  <div class="sidebar-section">
    <span class="sidebar-label">Penggunaan</span>
    <a href="#buttons">Peta Tombol</a>
    <a href="#modes">Mode Aplikasi</a>
    <a href="#viewfinder">Viewfinder</a>
    <a href="#capture">Ambil Foto</a>
    <a href="#format">Format Foto</a>
    <a href="#gallery">Gallery</a>
    <a href="#viewer">Photo Viewer</a>
    <a href="#video">Video / MJPEG</a>
    <a href="#face">Face Detect</a>
    <a href="#exposure">Exposure</a>
    <a href="#led">LED Flash</a>
    <a href="#usb">USB Mode</a>
  </div>
  <div class="sidebar-section">
    <span class="sidebar-label">Teknis</span>
    <a href="#stego">Steganografi</a>
    <a href="#exif">EXIF</a>
    <a href="#files">Struktur File</a>
    <a href="#changelog">Changelog</a>
    <a href="#troubleshoot">Troubleshoot</a>
  </div>
</aside>

<!-- CONTENT -->
<div class="content">

<!-- OVERVIEW -->
<section class="section" id="overview">
  <div class="section-header">
    <span class="section-num">01</span>
    <span class="section-title">Overview</span>
  </div>
  <p>SANZXCAM adalah firmware kamera penuh untuk <strong>Freenove ESP32-S3-WROOM</strong> dengan display ILI9341 2.4". Dibangun dengan UI bertema <strong>monokrom terminal</strong> — full hitam/abu/putih.</p>
  <div class="feature-grid">
    <div class="feature-card stego">
      <div class="fc-icon">◈ Steganografi</div>
      <div class="fc-title">Hidden Payload</div>
      <div class="fc-desc">Setiap foto menyimpan metadata tersembunyi. BMP via LSB pada byte Blue, JPG via COM marker header.</div>
    </div>
    <div class="feature-card face">
      <div class="fc-icon">◈ AI</div>
      <div class="fc-title">Face Detection</div>
      <div class="fc-desc">Real-time dual-model: MSR01 untuk kandidat awal, MNP01 untuk presisi + 5 keypoint wajah.</div>
    </div>
    <div class="feature-card rec">
      <div class="fc-icon">◈ Video</div>
      <div class="fc-title">MJPEG Record & Play</div>
      <div class="fc-desc">Rekam video langsung ke SD card format .mjpeg. Player built-in dengan speed control & loop.</div>
    </div>
    <div class="feature-card flash">
      <div class="fc-icon">◈ USB</div>
      <div class="fc-title">Mass Storage Mode</div>
      <div class="fc-desc">Mount SD card sebagai USB drive ke komputer host via tinyUSB tanpa perlu cabut SD.</div>
    </div>
  </div>
  <div class="tip info"><strong>Sensor yang didukung:</strong> GC2145 (BMP+JPG, XCLK 20MHz) dan OV3660 (JPG only, XCLK 24MHz). Sensor terdeteksi otomatis saat boot.</div>
</section>

<!-- HARDWARE -->
<section class="section" id="hardware">
  <div class="section-header">
    <span class="section-num">02</span>
    <span class="section-title">Hardware</span>
  </div>
  <table>
    <thead><tr><th>Komponen</th><th>Spesifikasi</th><th>Status</th></tr></thead>
    <tbody>
      <tr><td>Mikrokontroler</td><td>Freenove ESP32-S3-WROOM, 240MHz, 8MB PSRAM</td><td><span class="tag ok">Required</span></td></tr>
      <tr><td>Display</td><td>ILI9341 2.4" TFT 320×240, SPI + XPT2046 touch</td><td><span class="tag ok">Required</span></td></tr>
      <tr><td>Kamera</td><td>GC2145 atau OV3660 (QVGA 320×240)</td><td><span class="tag ok">Required</span></td></tr>
      <tr><td>MicroSD</td><td>FAT32, ≥4GB, SDMMC 1-bit mode</td><td><span class="tag ok">Required</span></td></tr>
      <tr><td>LED Indikator</td><td>GPIO 48 (built-in)</td><td><span class="tag">Built-in</span></td></tr>
      <tr><td>LED Flash</td><td>GPIO 2, dipasang eksternal</td><td><span class="tag warn">Optional</span></td></tr>
      <tr><td>Tombol</td><td>4× push button, active LOW, pull-up internal</td><td><span class="tag ok">Required</span></td></tr>
    </tbody>
  </table>
  <div class="tip warn"><strong>PSRAM wajib aktif.</strong> Tanpa PSRAM, alokasi buffer gallery (1000 file × 32 byte) dan pixel buffer foto akan gagal → device restart otomatis.</div>
</section>

<!-- PINOUT -->
<section class="section" id="pinout">
  <div class="section-header">
    <span class="section-num">03</span>
    <span class="section-title">Pinout</span>
  </div>
  <div class="pin-table-wrap">
    <div class="pin-group">
      <div class="pin-group-header">Kamera</div>
      <div class="pin-row"><span class="pin-sig">XCLK</span><span class="pin-num">15</span></div>
      <div class="pin-row"><span class="pin-sig">SIOD (SDA)</span><span class="pin-num">4</span></div>
      <div class="pin-row"><span class="pin-sig">SIOC (SCL)</span><span class="pin-num">5</span></div>
      <div class="pin-row"><span class="pin-sig">VSYNC</span><span class="pin-num">6</span></div>
      <div class="pin-row"><span class="pin-sig">HREF</span><span class="pin-num">7</span></div>
      <div class="pin-row"><span class="pin-sig">PCLK</span><span class="pin-num">13</span></div>
      <div class="pin-row"><span class="pin-sig">Y2–Y9</span><span class="pin-num">11,9,8,10,12,18,17,16</span></div>
    </div>
    <div class="pin-group">
      <div class="pin-group-header">Display ILI9341 + Touch XPT2046</div>
      <div class="pin-row"><span class="pin-sig">SPI MOSI</span><span class="pin-num">45</span></div>
      <div class="pin-row"><span class="pin-sig">SPI MISO</span><span class="pin-num">42</span></div>
      <div class="pin-row"><span class="pin-sig">SPI SCLK</span><span class="pin-num">47</span></div>
      <div class="pin-row"><span class="pin-sig">DC</span><span class="pin-num">14</span></div>
      <div class="pin-row"><span class="pin-sig">CS (display)</span><span class="pin-num">21</span></div>
      <div class="pin-row"><span class="pin-sig">RST</span><span class="pin-num">1</span></div>
    </div>
    <div class="pin-group">
      <div class="pin-group-header">SD Card (SDMMC 1-bit)</div>
      <div class="pin-row"><span class="pin-sig">CMD</span><span class="pin-num">38</span></div>
      <div class="pin-row"><span class="pin-sig">CLK</span><span class="pin-num">39</span></div>
      <div class="pin-row"><span class="pin-sig">D0</span><span class="pin-num">40</span></div>
    </div>
    <div class="pin-group">
      <div class="pin-group-header">Tombol & LED</div>
      <div class="pin-row"><span class="pin-sig">BTN_BOOT</span><span class="pin-num">0</span></div>
      <div class="pin-row"><span class="pin-sig">BTN_B</span><span class="pin-num">41</span></div>
      <div class="pin-row"><span class="pin-sig">BTN_C</span><span class="pin-num">3</span></div>
      <div class="pin-row"><span class="pin-sig">BTN_D</span><span class="pin-num">46</span></div>
      <div class="pin-row"><span class="pin-sig">LED (indikator)</span><span class="pin-num">48</span></div>
      <div class="pin-row"><span class="pin-sig">LED Flash</span><span class="pin-num">2</span></div>
    </div>
  </div>
</section>

<!-- INSTALL -->
<section class="section" id="install">
  <div class="section-header">
    <span class="section-num">04</span>
    <span class="section-title">Instalasi</span>
  </div>
  <h3>Library yang Dibutuhkan</h3>
  <pre><span class="cm">// Install via Arduino Library Manager atau PlatformIO</span>
LovyanGFX        <span class="am">>= 1.0.0</span>   <span class="cm">// display driver (LGFX_USE_V1)</span>
esp32-camera               <span class="cm">// esp_camera.h, img_converters.h</span>
JPEGDEC                    <span class="cm">// MJPEG frame decoder</span>
TJpg_Decoder               <span class="cm">// thumbnail & photo viewer decode</span>
MjpegClass                 <span class="cm">// MjpegClass.h — MJPEG player</span>
esp-dl                     <span class="cm">// human_face_detect_msr01/mnp01</span></pre>

  <h3>Konfigurasi Board</h3>
  <pre><span class="kw">Board   :</span> <span class="hi">ESP32S3 Dev Module</span>
<span class="kw">Flash   :</span> <span class="hi">16MB</span>
<span class="kw">PSRAM   :</span> <span class="hi">OPI PSRAM 8MB</span>
<span class="kw">Partition :</span> <span class="hi">16M Flash (3MB APP / 9.9MB FATFS)</span>
<span class="kw">Upload Speed :</span> <span class="hi">921600</span></pre>

  <h3>Langkah Upload</h3>
  <pre><span class="prompt">$</span> <span class="cm"># 1. Clone / download project</span>
<span class="prompt">$</span> git clone https://github.com/yourname/sanzxcam

<span class="prompt">$</span> <span class="cm"># 2. Buka .ino di Arduino IDE</span>
<span class="prompt">$</span> <span class="cm"># 3. Set board & library sesuai di atas</span>
<span class="prompt">$</span> <span class="cm"># 4. Upload</span>

<span class="prompt">$</span> <span class="cm"># 5. Monitor serial untuk boot log</span>
<span class="prompt">$</span> screen /dev/ttyUSB0 115200
<span class="ok">=== Sanzxcam v5.9-fix ===</span>
<span class="ok">[BOOT-GLITCH] boot animation diperpanjang ~2.5s</span></pre>
</section>

<!-- BOOT SEQUENCE -->
<section class="section" id="boot">
  <div class="section-header">
    <span class="section-num">05</span>
    <span class="section-title">Boot Sequence</span>
  </div>
  <p>Dua fase animasi boot saat power-on. Total durasi Fase 1 ~2.5 detik (diperbarui dari v5.9).</p>

  <h3>Fase 1 — Splash Glitch</h3>
  <pre><span class="cm">[ teks acak 8 char ]</span>
       ↓  glitch bars flash
<span class="am">scramble karakter satu-satu → "SANZXCAM"</span>
       ↓  chromatic aberration (ghost merah & biru)
<span class="hi">SANZXCAM</span>  ← teks utama normal
       ↓  400ms hold
<span class="cm">subtitle + versi muncul</span>
       ↓  600ms → glitch akhir → 800ms
<span class="cm">wipe stripe ke hitam (10ms per stripe)</span></pre>

  <h3>Fase 2 — Boot Log</h3>
  <div class="terminal">
    <div class="terminal-header">
      <div class="term-dots"><div class="term-dot"></div><div class="term-dot"></div><div class="term-dot"></div></div>
      <div class="term-title">boot sequence — SANZXCAM v5.9-fix</div>
    </div>
    <div class="terminal-body">
      <div class="t-section">── CAMERA ─────────────────────────────</div>
      <div class="t-row"><span class="t-key">SENSOR PID</span><span class="t-dot">·········</span><span class="t-val-ok">0x2145  GC2145</span></div>
      <div class="t-row"><span class="t-key">XCLK</span><span class="t-dot">·············</span><span class="t-val-ok">20MHz  OK</span></div>
      <div class="t-row"><span class="t-key">RESOLUTION</span><span class="t-dot">·········</span><span class="t-val-dim">320x240  QVGA</span></div>
      <div class="t-row"><span class="t-key">FORMAT</span><span class="t-dot">···········</span><span class="t-val-dim">RGB565  BMP</span></div>
      <div class="t-section">── STORAGE ────────────────────────────</div>
      <div class="t-row"><span class="t-key">SD CARD</span><span class="t-dot">··········</span><span class="t-val-ok">OK  3814MB</span></div>
      <div class="t-row"><span class="t-key">FILESYSTEM</span><span class="t-dot">·······</span><span class="t-val-dim">FAT32</span></div>
      <div class="t-row"><span class="t-key">PHOTOS</span><span class="t-dot">···········</span><span class="t-val-dim">0042 files</span></div>
      <div class="t-section">── SYSTEM ─────────────────────────────</div>
      <div class="t-row"><span class="t-key">CPU FREQ</span><span class="t-dot">·········</span><span class="t-val-ok">240MHz  OK</span></div>
      <div class="t-row"><span class="t-key">PSRAM</span><span class="t-dot">············</span><span class="t-val-ok">8MB  OK</span></div>
      <div class="t-row"><span class="t-key">HEAP FREE</span><span class="t-dot">·······</span><span class="t-val-dim">312KB</span></div>
      <div class="t-section">── BUILD ──────────────────────────────</div>
      <div class="t-row"><span class="t-key">VERSION</span><span class="t-dot">··········</span><span class="t-val-dim">v5.9-fix</span></div>
      <div class="t-row"><span class="t-key">STEGO-BMP</span><span class="t-dot">·······</span><span class="t-val-ok">embed in saveBMP OK</span></div>
      <div class="t-row"><span class="t-key">FEATURES</span><span class="t-dot">········</span><span class="t-val-dim">EXIF+STEGO+FACE+REC</span></div>
      <div class="t-bar">──────────────────────────────────────</div>
      <div class="t-bar">[████████████████████████████████] </div>
      <div class="t-ready">R E A D Y</div>
    </div>
  </div>
</section>

<!-- BUTTON MAP -->
<section class="section" id="buttons">
  <div class="section-header">
    <span class="section-num">06</span>
    <span class="section-title">Peta Tombol</span>
  </div>
  <p>Semua tombol mendukung <strong>short press</strong> (&lt;1500ms) dan <strong>long press</strong> (≥1500ms). Debounce 80ms.</p>

  <h3>Mode Viewfinder (layar utama)</h3>
  <div class="btn-grid">
    <div class="btn-card">
      <div class="btn-card-header">
        <div class="btn-chip boot">BOOT</div>
        <div class="btn-label">GPIO 0</div>
      </div>
      <div class="btn-card-body">
        <div class="btn-action"><span class="btn-press">short</span><span class="btn-desc">Ambil foto (BMP atau JPG)</span></div>
        <div class="btn-action"><span class="btn-press">long</span><span class="btn-desc">Masuk USB Mass Storage Mode (SD harus tersedia)</span></div>
      </div>
    </div>
    <div class="btn-card">
      <div class="btn-card-header">
        <div class="btn-chip b">BTN_B</div>
        <div class="btn-label">GPIO 41</div>
      </div>
      <div class="btn-card-body">
        <div class="btn-action"><span class="btn-press">short</span><span class="btn-desc">Mulai / Stop recording video MJPEG</span></div>
        <div class="btn-action"><span class="btn-press">long</span><span class="btn-desc">Buka menu LED Flash (on/off)</span></div>
      </div>
    </div>
    <div class="btn-card">
      <div class="btn-card-header">
        <div class="btn-chip c">BTN_C</div>
        <div class="btn-label">GPIO 3</div>
      </div>
      <div class="btn-card-body">
        <div class="btn-action"><span class="btn-press">short</span><span class="btn-desc">Buka Gallery (SD harus tersedia)</span></div>
        <div class="btn-action"><span class="btn-press">long</span><span class="btn-desc">Menu pilih format foto (GC2145: BMP/JPG)</span></div>
      </div>
    </div>
    <div class="btn-card">
      <div class="btn-card-header">
        <div class="btn-chip d">BTN_D</div>
        <div class="btn-label">GPIO 46</div>
      </div>
      <div class="btn-card-body">
        <div class="btn-action"><span class="btn-press">short</span><span class="btn-desc">Toggle Face Detection on/off</span></div>
        <div class="btn-action"><span class="btn-press">long</span><span class="btn-desc">Buka menu Exposure (AUTO/MOON/NIGHT/MANUAL)</span></div>
      </div>
    </div>
  </div>

  <h3>Semua Mode — Referensi Lengkap</h3>
  <div class="mode-flow">
    <div class="mode-row">
      <div class="mode-name">VIEWFINDER</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT short</span><span class="key-action">Ambil foto → simpan BMP atau JPG ke SD</span></div>
        <div class="mode-key-row"><span class="key-combo">BOOT long</span><span class="key-action">Masuk USB Mass Storage Mode</span></div>
        <div class="mode-key-row"><span class="key-combo">B short</span><span class="key-action">Start/Stop recording video .mjpeg</span></div>
        <div class="mode-key-row"><span class="key-combo">B long</span><span class="key-action">Menu LED Flash</span></div>
        <div class="mode-key-row"><span class="key-combo">C short</span><span class="key-action">Buka Gallery</span></div>
        <div class="mode-key-row"><span class="key-combo">C long</span><span class="key-action">Menu format foto (GC2145 only)</span></div>
        <div class="mode-key-row"><span class="key-combo">D short</span><span class="key-action">Toggle Face Detection</span></div>
        <div class="mode-key-row"><span class="key-combo">D long</span><span class="key-action">Menu Exposure preset</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">GALLERY</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT short</span><span class="key-action">Buka file terpilih (foto/video)</span></div>
        <div class="mode-key-row"><span class="key-combo">B short</span><span class="key-action">Kembali ke Viewfinder</span></div>
        <div class="mode-key-row"><span class="key-combo">C tahan</span><span class="key-action">Scroll ke atas (akselerasi: 200→100→50ms)</span></div>
        <div class="mode-key-row"><span class="key-combo">D tahan</span><span class="key-action">Scroll ke bawah (akselerasi: 200→100→50ms)</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">PHOTO VIEW<br><small style="font-size:9px;color:#555">zoom 1×</small></div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT short</span><span class="key-action">Kembali ke Gallery</span></div>
        <div class="mode-key-row"><span class="key-combo">B short</span><span class="key-action">Zoom in (1× → 2× → 4× → 1× siklus)</span></div>
        <div class="mode-key-row"><span class="key-combo">C short</span><span class="key-action">Foto sebelumnya</span></div>
        <div class="mode-key-row"><span class="key-combo">D short</span><span class="key-action">Foto berikutnya</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">PHOTO VIEW<br><small style="font-size:9px;color:#555">zoom 2×/4×</small></div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT short</span><span class="key-action">Kembali ke Gallery</span></div>
        <div class="mode-key-row"><span class="key-combo">B short</span><span class="key-action">Zoom (siklus level)</span></div>
        <div class="mode-key-row"><span class="key-combo">B long</span><span class="key-action">Buka dialog hapus file</span></div>
        <div class="mode-key-row"><span class="key-combo">C short</span><span class="key-action">Pan kiri (step 20px)</span></div>
        <div class="mode-key-row"><span class="key-combo">C long</span><span class="key-action">Pan atas (step 20px)</span></div>
        <div class="mode-key-row"><span class="key-combo">D short</span><span class="key-action">Pan kanan (step 20px)</span></div>
        <div class="mode-key-row"><span class="key-combo">D long</span><span class="key-action">Pan bawah (step 20px)</span></div>
        <div class="mode-key-row"><span class="key-combo">C/D tahan</span><span class="key-action">Pan kontinyu (interval 120ms)</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">MJPEG PLAYER</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Stop & kembali ke Gallery</span></div>
        <div class="mode-key-row"><span class="key-combo">B</span><span class="key-action">Pause / Resume playback</span></div>
        <div class="mode-key-row"><span class="key-combo">C</span><span class="key-action">Toggle LOOP on/off</span></div>
        <div class="mode-key-row"><span class="key-combo">D</span><span class="key-action">Ganti kecepatan: 0.5× → 1.0× → 2.0×</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">MENU LED</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Konfirmasi pilihan flash</span></div>
        <div class="mode-key-row"><span class="key-combo">B</span><span class="key-action">Batal, kembali ke Viewfinder</span></div>
        <div class="mode-key-row"><span class="key-combo">C / D</span><span class="key-action">Toggle LED ON / LED OFF</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">MENU EXPOSURE</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Terapkan preset (MANUAL → buka adj panel)</span></div>
        <div class="mode-key-row"><span class="key-combo">B</span><span class="key-action">Batal, kembali ke Viewfinder</span></div>
        <div class="mode-key-row"><span class="key-combo">C</span><span class="key-action">Pilih preset sebelumnya (loop)</span></div>
        <div class="mode-key-row"><span class="key-combo">D</span><span class="key-action">Pilih preset berikutnya (loop)</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">EXP MANUAL ADJ</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Simpan & kembali ke Viewfinder</span></div>
        <div class="mode-key-row"><span class="key-combo">C</span><span class="key-action">EXP − (step adaptif: 10/25/50)</span></div>
        <div class="mode-key-row"><span class="key-combo">D</span><span class="key-action">EXP + (step adaptif: 10/25/50)</span></div>
        <div class="mode-key-row"><span class="key-combo">B</span><span class="key-action">GAIN + 1 (siklus 0–30)</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">MENU FORMAT</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Konfirmasi pilihan format</span></div>
        <div class="mode-key-row"><span class="key-combo">B</span><span class="key-action">Batal, kembali ke Viewfinder</span></div>
        <div class="mode-key-row"><span class="key-combo">C / D</span><span class="key-action">Toggle BMP / JPG</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">DIALOG HAPUS</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Konfirmasi hapus file</span></div>
        <div class="mode-key-row"><span class="key-combo">B / C / D</span><span class="key-action">Batal, kembali ke Photo View</span></div>
        <div class="mode-key-row"><span class="key-combo">timeout 8s</span><span class="key-action">Otomatis batal (progress bar countdown)</span></div>
      </div>
    </div>
    <div class="mode-row">
      <div class="mode-name">USB MODE</div>
      <div class="mode-keys">
        <div class="mode-key-row"><span class="key-combo">BOOT</span><span class="key-action">Keluar USB mode, remount SD ke VFS</span></div>
        <div class="mode-key-row"><span class="key-combo">—</span><span class="key-action">Semua tombol lain diabaikan</span></div>
      </div>
    </div>
  </div>
</section>

<!-- MODES -->
<section class="section" id="modes">
  <div class="section-header">
    <span class="section-num">07</span>
    <span class="section-title">Mode Aplikasi</span>
  </div>
  <pre><span class="hi">MODE_VIEWFINDER</span>    <span class="cm">← startup default</span>
  ↓ C short        → <span class="hi">MODE_GALLERY</span>
  ↓ B long         → <span class="hi">MODE_MENU_LED</span>
  ↓ C long         → <span class="hi">MODE_MENU_FORMAT</span>    <span class="cm">(GC2145 only)</span>
  ↓ D long         → <span class="hi">MODE_MENU_EXP</span>
  ↓ BOOT long      → <span class="am">USB Mode</span>

<span class="hi">MODE_GALLERY</span>
  ↓ BOOT (foto)    → <span class="hi">MODE_PHOTO_VIEW</span>
  ↓ BOOT (video)   → <span class="hi">MODE_MJPEG_PLAYER</span>
  ↓ B              → MODE_VIEWFINDER

<span class="hi">MODE_PHOTO_VIEW</span>
  ↓ B long         → <span class="hi">MODE_DIALOG_DELETE</span>
  ↓ BOOT           → MODE_GALLERY

<span class="hi">MODE_MENU_EXP</span>
  ↓ BOOT (MANUAL)  → <span class="hi">MODE_MENU_EXP_ADJ</span></pre>
</section>

<!-- VIEWFINDER -->
<section class="section" id="viewfinder">
  <div class="section-header">
    <span class="section-num">08</span>
    <span class="section-title">Viewfinder</span>
  </div>
  <p>Live feed kamera 320×240 dengan overlay informasi dan Dynamic Island di atas layar.</p>
  <pre><span class="cm">┌─────────────────────────────────────────────────┐</span>
<span class="cm">│</span> <span class="am">[  14 fps ]</span>  <span class="hi">[ FACE  2 ]</span>  <span class="am">[ GC2145 * ]</span>         <span class="cm">│</span>
<span class="cm">│</span>                                                  <span class="cm">│</span>
<span class="cm">│</span>     <span class="cm">┌ ─ ─ ─ ─ ─ ─ live feed ─ ─ ─ ─ ─ ─ ┐</span>     <span class="cm">│</span>
<span class="cm">│</span>     <span class="cm">│</span>          320 × 240 RGB565           <span class="cm">│</span>     <span class="cm">│</span>
<span class="cm">│</span>     <span class="cm">└ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘</span>     <span class="cm">│</span>
<span class="cm">│</span>                                                  <span class="cm">│</span>
<span class="cm">│</span> <span class="am">[ #0043 BMP ]</span>                  <span class="ok">[ SD  OK ]</span>       <span class="cm">│</span>
<span class="cm">└─────────────────────────────────────────────────┘</span></pre>
  <table>
    <thead><tr><th>Elemen</th><th>Posisi</th><th>Isi</th></tr></thead>
    <tbody>
      <tr><td>FPS Pill</td><td>kiri atas</td><td>Framerate rata-rata bergerak (0.7/0.3 weight)</td></tr>
      <tr><td>Mode Pill</td><td>tengah atas</td><td><code>FACE N</code> / <code>MOON</code> / <code>NIGHT</code> / <code>M 850</code> (manual)</td></tr>
      <tr><td>Sensor Pill</td><td>kanan atas</td><td>Nama sensor + <code> *</code> jika flash aktif</td></tr>
      <tr><td>Shot Counter</td><td>kiri bawah</td><td><code>#0043 BMP</code> — nomor foto berikutnya + format</td></tr>
      <tr><td>SD Status</td><td>kanan bawah</td><td><code>SD  OK</code> hijau / <code>SD  --</code> abu jika tidak ada SD</td></tr>
      <tr><td>Corner Brackets</td><td>4 sudut</td><td>Abu = normal, Merah = recording, Putih = face detect</td></tr>
      <tr><td>REC Indicator</td><td>kiri atas (saat rec)</td><td>Lingkaran blink + timer MM:SS + jumlah frame</td></tr>
    </tbody>
  </table>
</section>

<!-- CAPTURE -->
<section class="section" id="capture">
  <div class="section-header">
    <span class="section-num">09</span>
    <span class="section-title">Ambil Foto</span>
  </div>
  <p>Tekan <code>BOOT short</code> di Viewfinder. Format simpan tergantung sensor dan pengaturan.</p>
  <pre><span class="cm">Alur Capture:</span>

<span class="ok">1.</span> Jika flash aktif → LED_FLASH HIGH (GPIO 2)
   → delay 150ms → flush 2 frame → kamera stabil
<span class="ok">2.</span> Ambil frame: esp_camera_fb_get()
   → LED_FLASH LOW
<span class="ok">3.</span> Preview frame sekilas di layar
<span class="ok">4.</span> Cek sensor & format:

   <span class="hi">GC2145 + RGB565 + format BMP?</span>
     → saveBMP() dengan stego LSB embed
     → simpan: /sdcard/photo_XXXX.bmp

   <span class="hi">lainnya (OV3660 / format JPG)?</span>
     → frame2jpg(quality=85)
     → stegoEmbedToJpeg() → COM marker
     → exifInjectToJpeg() → APP1 EXIF
     → simpan: /sdcard/photo_XXXX.jpg

<span class="ok">5.</span> Notifikasi Dynamic Island: "SAVED BMP #0043"
<span class="ok">6.</span> LED blink 2× konfirmasi</pre>

  <div class="tip warn"><strong>Saat recording video berlangsung</strong>, hanya BTN_B yang diproses (untuk stop). Semua tombol lain diabaikan termasuk BOOT/capture.</div>
</section>

<!-- FORMAT -->
<section class="section" id="format">
  <div class="section-header">
    <span class="section-num">10</span>
    <span class="section-title">Format Foto</span>
  </div>
  <p>Buka dengan <code>BTN_C long press</code> di Viewfinder (GC2145 only). OV3660 selalu JPG.</p>
  <table>
    <thead><tr><th>Aspek</th><th>BMP</th><th>JPG</th></tr></thead>
    <tbody>
      <tr><td>Kompresi</td><td>Tidak (lossless raw)</td><td>Lossy quality 85</td></tr>
      <tr><td>Warna</td><td>BGR888 asli tanpa kompresi</td><td>YCbCr terkompresi</td></tr>
      <tr><td>Ukuran</td><td>~225 KB (320×240×3)</td><td>~30–80 KB</td></tr>
      <tr><td>Stego</td><td>LSB byte Blue BGR888</td><td>COM marker (0xFE)</td></tr>
      <tr><td>Metadata</td><td>—</td><td>APP1 EXIF (Make, Model, Software, DateTime)</td></tr>
      <tr><td>Sensor</td><td>GC2145 saja</td><td>GC2145 + OV3660</td></tr>
    </tbody>
  </table>
</section>

<!-- GALLERY -->
<section class="section" id="gallery">
  <div class="section-header">
    <span class="section-num">11</span>
    <span class="section-title">Gallery</span>
  </div>
  <p>Scan semua <code>.jpg</code>, <code>.bmp</code>, <code>.mjpeg</code> (case-insensitive) dari SD card, sort alfabetis, tampil 8 item per halaman.</p>
  <pre><span class="cm">┌────────────────────────── GALLERY  58 item ──────────────────┐</span>
<span class="cm">│</span>  <span class="cm">1/8</span>                                          <span class="cm">B=BACK</span>       <span class="cm">│</span>
<span class="cm">├──────────────────────────────────────────────────────────────┤</span>
<span class="cm">│</span>  <span class="cm"> 1</span>  <span class="cm">□</span>  photo_0001.jpg                              <span class="am">JPG</span>   <span class="cm">│</span>
<span class="cm">│</span>  <span class="cm"> 2</span>  <span class="ok">B</span>  photo_0002.bmp                              <span class="ok">BMP</span>   <span class="cm">│</span>
<span class="cm">│</span>  <span class="cm"> 3</span>  <span class="hi">▶</span>  video_0001.mjpeg                            <span class="hi">VID</span>   <span class="cm">│</span>
<span class="cm">│</span>  <span class="cm"> 4</span>  <span class="cm">□</span>  photo_0003.jpg                              <span class="am">JPG</span>   <span class="cm">│</span>
<span class="cm">│ ┌────────────────────────────────────────────────────────┐  │</span>
<span class="cm">│ │ ←── selection highlight ──────────────────────────────┘  │</span>
<span class="cm">└──────────────────────────────────────────────────────────────┘</span></pre>
  <div class="tip"><strong>Scroll akselerasi:</strong> Tahan C/D → awal 200ms/step → setelah 500ms jadi 100ms → setelah 1500ms jadi 50ms. Rilis tombol untuk reset kecepatan.</div>
</section>

<!-- VIEWER -->
<section class="section" id="viewer">
  <div class="section-header">
    <span class="section-num">12</span>
    <span class="section-title">Photo Viewer</span>
  </div>
  <p>Buka foto dari gallery. Support BMP (via <code>loadBMP()</code>) dan JPG (via TJpgDec). Zoom 3 level dengan nearest-neighbor scaling.</p>
  <table>
    <thead><tr><th>Zoom Level</th><th>Faktor</th><th>Pan Step</th><th>Mode</th></tr></thead>
    <tbody>
      <tr><td>Level 0</td><td>1×</td><td>—</td><td>Navigasi foto prev/next</td></tr>
      <tr><td>Level 1</td><td>2×</td><td>20px</td><td>Pan mode, C/D short/long = kiri/kanan/atas/bawah</td></tr>
      <tr><td>Level 2</td><td>4×</td><td>20px</td><td>Pan mode</td></tr>
    </tbody>
  </table>
  <p>Caption bar (bawah, 2 detik saat foto dibuka):</p>
  <pre><span class="cm">< 5 / 23 >  photo_0005.bmp [BMP] [#5 v5.9]</span>
               <span class="cm">↑              ↑        ↑</span>
         <span class="cm">nama file    tipe    stego payload</span></pre>
</section>

<!-- VIDEO -->
<section class="section" id="video">
  <div class="section-header">
    <span class="section-num">13</span>
    <span class="section-title">Video MJPEG</span>
  </div>
  <h3>Recording</h3>
  <p>Format: raw MJPEG stream — frame JPEG dikoncatenasi langsung ke file. Nama file <code>video_XXXX.mjpeg</code>.</p>
  <table>
    <thead><tr><th>Parameter</th><th>Nilai</th></tr></thead>
    <tbody>
      <tr><td>Format container</td><td>.mjpeg (raw stream)</td></tr>
      <tr><td>Frame encoding</td><td>JPEG quality 70 (dari RGB565 via frame2jpg)</td></tr>
      <tr><td>Resolusi</td><td>320×240</td></tr>
      <tr><td>Frame rate</td><td>~10–20 fps (tergantung write speed SD)</td></tr>
      <tr><td>Buffer player</td><td>150 KB</td></tr>
      <tr><td>Target playback fps</td><td>15 fps × speed multiplier</td></tr>
    </tbody>
  </table>

  <h3>MJPEG Player</h3>
  <p>Timing presisi via <code>esp_timer_get_time()</code> dengan busy-wait microsecond.</p>
  <pre><span class="cm">targetUs = 1_000_000 / (15fps × speed)</span>
<span class="cm">remain   = targetUs - elapsed</span>
<span class="cm">if remain > 1000µs → busy-wait 500µs steps</span>

<span class="cm">Speed 0.5× → 133ms/frame  (7.5 fps)</span>
<span class="cm">Speed 1.0× →  66ms/frame  (15 fps)</span>
<span class="cm">Speed 2.0× →  33ms/frame  (30 fps)</span></pre>
</section>

<!-- FACE -->
<section class="section" id="face">
  <div class="section-header">
    <span class="section-num">14</span>
    <span class="section-title">Face Detection</span>
  </div>
  <p>Dual-model inference pada setiap frame viewfinder saat aktif. Toggle dengan <code>BTN_D short</code>.</p>
  <pre><span class="cm">Frame RGB565 →</span>
  <span class="hi">HumanFaceDetectMSR01</span>(0.1, 0.5, 10, 0.2)
    <span class="cm">↓ candidates[]</span>
  <span class="hi">HumanFaceDetectMNP01</span>(0.5, 0.3, 5)
    <span class="cm">↓ results[]{box[4], keypoint[10]}</span>
  <span class="cm">→ gambar corner brackets per wajah</span>
  <span class="cm">→ gambar 5 keypoint (2 mata, hidung, 2 sudut mulut)</span></pre>
  <div class="tip warn"><strong>Face detect otomatis off</strong> saat recording dimulai (<code>startRecording()</code> menonaktifkan mode ini untuk menjaga framerate write ke SD).</div>
</section>

<!-- EXPOSURE -->
<section class="section" id="exposure">
  <div class="section-header">
    <span class="section-num">15</span>
    <span class="section-title">Exposure Preset</span>
  </div>
  <table>
    <thead><tr><th>Preset</th><th>AEC</th><th>AEC2</th><th>AEC Value</th><th>Gain</th><th>AE Level</th><th>Cocok untuk</th></tr></thead>
    <tbody>
      <tr><td><code>AUTO</code></td><td><span class="tag ok">ON</span></td><td><span class="tag ok">ON</span></td><td>300 (auto)</td><td>AUTO</td><td>0</td><td>Normal, pencahayaan cukup</td></tr>
      <tr><td><code>MOON</code></td><td><span class="tag err">OFF</span></td><td><span class="tag err">OFF</span></td><td>20</td><td>OFF (0)</td><td>-2</td><td>Objek sangat terang, bulan</td></tr>
      <tr><td><code>NIGHT</code></td><td><span class="tag err">OFF</span></td><td><span class="tag ok">ON</span></td><td>1200</td><td>OFF (5)</td><td>+2</td><td>Kondisi gelap, malam</td></tr>
      <tr><td><code>MANUAL</code></td><td><span class="tag err">OFF</span></td><td><span class="tag err">OFF</span></td><td>0–1200</td><td>OFF (0–30)</td><td>0</td><td>Kontrol penuh manual</td></tr>
    </tbody>
  </table>
  <div class="tip"><strong>Mode MANUAL</strong> membuka sub-panel overlay di bawah layar dengan live preview kamera. Step EXP adaptif: &lt;100 = step 10, &lt;400 = step 25, ≥400 = step 50. Setiap perubahan langsung diterapkan ke sensor + flush 2 frame.</div>
</section>

<!-- LED -->
<section class="section" id="led">
  <div class="section-header">
    <span class="section-num">16</span>
    <span class="section-title">LED Flash</span>
  </div>
  <pre><span class="cm">BTN_B long → Menu LED</span>

<span class="cm">Saat flash aktif (capture):</span>
<span class="ok">1.</span> LED_FLASH (GPIO 2) → HIGH
<span class="ok">2.</span> delay 150ms
<span class="ok">3.</span> flush 2 frame (kamera stabilisasi eksposur)
<span class="ok">4.</span> ambil frame ke-3 sebagai foto
<span class="ok">5.</span> LED_FLASH → LOW

<span class="cm">Indikator aktif: "GC2145 *" di sensor pill viewfinder</span></pre>
</section>

<!-- USB -->
<section class="section" id="usb">
  <div class="section-header">
    <span class="section-num">17</span>
    <span class="section-title">USB Mass Storage</span>
  </div>
  <pre><span class="cm">BOOT long press (>1.5s) di Viewfinder:</span>

<span class="ok">1.</span> unmountVFSOnly() → VFS di-unmount dari ESP32
<span class="ok">2.</span> msc.mediaPresent(true) → expose ke host via tinyUSB
<span class="ok">3.</span> Layar tampilkan status: sensor, kapasitas, jumlah foto
<span class="ok">4.</span> sdReady = false (ESP32 tidak akses SD langsung)

<span class="cm">Host baca/tulis via onRead() / onWrite():</span>
  sdmmc_read_sectors() / sdmmc_write_sectors()

<span class="cm">Keluar (BOOT di layar USB):</span>
<span class="ok">1.</span> msc.mediaPresent(false)
<span class="ok">2.</span> remountVFSOnly() → f_mount + esp_vfs_fat_register
<span class="ok">3.</span> scanPhotoCount()
<span class="ok">4.</span> kembali ke MODE_VIEWFINDER</pre>
  <div class="tip warn"><strong>Penting:</strong> Selalu eject dari host terlebih dahulu sebelum tekan BOOT untuk keluar. Jika tidak, filesystem bisa korup.</div>
</section>

<!-- STEGO -->
<section class="section" id="stego">
  <div class="section-header">
    <span class="section-num">18</span>
    <span class="section-title">Steganografi</span>
  </div>

  <h3>JPEG — COM Marker (0xFE)</h3>
  <p>Payload diembed sebagai COM segment tepat setelah SOI di awal file JPEG.</p>
  <pre><span class="cm">Format payload:</span>  <span class="ok">SANZXCAM|0042|v5.9</span>
<span class="cm">Max length:</span> 32 byte  |  <span class="cm">Magic:</span> "SANZXCAM"

<span class="cm">Struktur file JPG setelah embed:</span>
<span class="hi">FF D8</span>         <span class="cm">← SOI</span>
<span class="hi">FF E1</span> ...     <span class="cm">← APP1 EXIF</span>
<span class="hi">FF FE</span> LL PP   <span class="cm">← COM marker (stego payload)</span>
<span class="hi">FF E0</span> ...     <span class="cm">← sisa JPEG data</span></pre>

  <pre><span class="cm"># Ekstraksi manual dengan Python:</span>
<span class="kw">with</span> <span class="fn">open</span>(<span class="st">"photo_0042.jpg"</span>, <span class="st">"rb"</span>) <span class="kw">as</span> f:
    data = f.<span class="fn">read</span>()
pos = <span class="hi">2</span>
<span class="kw">while</span> pos &lt; <span class="fn">len</span>(data) - <span class="hi">3</span>:
    <span class="kw">if</span> data[pos] == <span class="hi">0xFF</span> <span class="kw">and</span> data[pos+<span class="hi">1</span>] == <span class="hi">0xFE</span>:
        length = (data[pos+<span class="hi">2</span>] &lt;&lt; <span class="hi">8</span>) | data[pos+<span class="hi">3</span>]
        payload = data[pos+<span class="hi">4</span>:pos+<span class="hi">2</span>+length].<span class="fn">decode</span>()
        <span class="fn">print</span>(payload)  <span class="cm"># SANZXCAM|0042|v5.9</span>
        <span class="kw">break</span>
    <span class="cm"># ... skip marker lain</span></pre>

  <h3>BMP — LSB Blue Channel (v5.9-fix)</h3>
  <p>Payload diembed ke bit-0 (LSB) byte Blue pada setiap pixel BGR888. Embed dilakukan langsung di dalam <code>saveBMP()</code>.</p>
  <pre><span class="cm">Payload:</span>  <span class="ok">SANZXCAM|0042|v5.9</span>
<span class="cm">Max length:</span> 64 byte  |  <span class="cm">Kapasitas:</span> 320×240 = 76800 pixel

<span class="cm">Algoritma embed (per pixel, iterasi y=H-1..0 bottom-up):</span>
  charIdx = stegoPixelIdx / 8
  bitPos  = 7 - (stegoPixelIdx % 8)   <span class="cm">← MSB dulu</span>
  bit     = payload[charIdx] >> bitPos) & 1
  B_final = (B & 0xFE) | bit           <span class="cm">← set LSB</span>

<span class="cm">Algoritma extract (stegoBmpExtract):</span>
  baca file BMP row by row bottom-up (y=H-1..0)
  tiap pixel: bit = rowBuf[x*3+0] & 0x01
  susun 8 bit → 1 char → string payload
  validasi magic "SANZXCAM"</pre>

  <div class="tip ok"><strong>Bugfix v5.9-fix:</strong> Bug lama embed ke <code>rgb565Buf[p*2+1]</code> menyebabkan bit bergeser 3 posisi setelah konversi RGB565→BGR888 (<code>&lt;&lt;3</code>). Solusi: embed langsung ke byte BGR888 final di dalam <code>saveBMP()</code>, bukan sebelum konversi. Urutan pixel juga diselaraskan (bottom-up) antara embed dan extract.</div>
</section>

<!-- EXIF -->
<section class="section" id="exif">
  <div class="section-header">
    <span class="section-num">19</span>
    <span class="section-title">EXIF Inject</span>
  </div>
  <p>Setiap foto JPG mendapat APP1 EXIF yang dibangun manual (little-endian TIFF IFD).</p>
  <table>
    <thead><tr><th>Tag</th><th>ID</th><th>Nilai</th></tr></thead>
    <tbody>
      <tr><td>ImageDescription</td><td>0x010E</td><td><code>photo_0042</code></td></tr>
      <tr><td>Make</td><td>0x010F</td><td><code>SANZXCAM</code></td></tr>
      <tr><td>Model</td><td>0x0110</td><td><code>GC2145</code> / <code>OV3660</code></td></tr>
      <tr><td>Software</td><td>0x0131</td><td><code>v5.9</code></td></tr>
      <tr><td>ExifVersion</td><td>0x9000</td><td><code>0220</code></td></tr>
      <tr><td>DateTimeOriginal</td><td>0x9003</td><td><code>2025:01:01 HH:MM:SS</code> (dari uptime)</td></tr>
    </tbody>
  </table>
  <pre><span class="prompt">$</span> exiftool photo_0042.jpg
<span class="ok">Image Description               : photo_0042</span>
<span class="ok">Make                            : SANZXCAM</span>
<span class="ok">Camera Model Name               : GC2145</span>
<span class="ok">Software                        : v5.9</span>
<span class="ok">Date/Time Original              : 2025:01:01 00:07:23</span></pre>
</section>

<!-- FILE STRUCTURE -->
<section class="section" id="files">
  <div class="section-header">
    <span class="section-num">20</span>
    <span class="section-title">Struktur File SD</span>
  </div>
  <pre>/sdcard/
├── <span class="ok">photo_0001.bmp</span>      <span class="cm">← BMP 24-bit, stego LSB</span>
├── <span class="am">photo_0002.jpg</span>      <span class="cm">← JPG, EXIF APP1 + stego COM</span>
├── <span class="ok">photo_0003.bmp</span>
├── <span class="am">photo_0004.jpg</span>
├── ...
├── <span class="hi">video_0001.mjpeg</span>   <span class="cm">← raw MJPEG stream</span>
├── <span class="hi">video_0002.mjpeg</span>
└── ...</pre>
  <p>Penomoran foto via <code>scanPhotoCount()</code>: scan semua <code>photo_XXXX.bmp</code>/<code>.jpg</code>, ambil nomor terbesar. Penomoran video via <code>scanVideoCount()</code> serupa.</p>
</section>

<!-- CHANGELOG -->
<section class="section" id="changelog">
  <div class="section-header">
    <span class="section-num">21</span>
    <span class="section-title">Changelog</span>
  </div>

  <div class="cl-entry fix">
    <div class="cl-tag fix">FIX — v5.9-fix</div>
    <div class="cl-title">FIX-STEGO-BMP — Perbaikan Steganografi BMP (2 Bug)</div>
    <div class="cl-body">
      <p><strong>Bug 1 — Bit loss saat RGB565 → BGR888</strong></p>
      <p>Versi lama embed ke LSB <code>rgb565Buf[p*2+1]</code>. Fungsi <code>saveBMP()</code> expand: <code>b = (px &amp; 0x1F) &lt;&lt; 3</code> — shift 3 bit kiri. Bit yang di-embed bergeser ke posisi bit-3 pada BGR888, bukan bit-0 yang dibaca extractor.</p>
      <p><strong>Bug 2 — Urutan pixel terbalik</strong></p>
      <p>Embed lama iterasi <code>p=0,1,...</code> = baris <code>y=0,1,...</code> dari atas. BMP ditulis bottom-up: <code>y=H-1</code> ditulis pertama. Extractor baca dari <code>row=bmpH-1</code> turun ke 0. Urutan pixel embed ≠ urutan extract → payload korup.</p>
      <p><strong>Solusi:</strong> Hapus <code>stegoBmpEmbed()</code> terpisah. Embed langsung di dalam <code>saveBMP()</code> pada byte Blue BGR888 final, loop <code>y=H-1..0</code>. Urutan identik dengan extractor.</p>
    </div>
  </div>

  <div class="cl-entry new">
    <div class="cl-tag new">UPDATE — v5.9-fix</div>
    <div class="cl-title">BOOT-GLITCH — Durasi Animasi Diperpanjang ~1.2s → ~2.5s</div>
    <div class="cl-body">
      <ul>
        <li>Scramble text step delay: <code>30ms</code> → <code>50ms</code> (+~160ms total)</li>
        <li>Jeda setelah teks normal: <code>0ms</code> → <code>400ms</code></li>
        <li>Jeda sebelum glitch akhir: <code>100ms</code> → <code>600ms</code></li>
        <li>Jeda setelah restore final: <code>300ms</code> → <code>800ms</code></li>
        <li>Stripe wipe delay: <code>4ms</code> → <code>10ms</code> (lebih smooth, +~180ms)</li>
      </ul>
    </div>
  </div>

  <div class="cl-entry fix">
    <div class="cl-tag fix">DARI v5.9</div>
    <div class="cl-title">Fitur yang Dipertahankan dari v5.9</div>
    <div class="cl-body">
      <ul>
        <li>FIX-BMP-COLOR: byte-swap RGB565 sebelum konversi ke BGR888</li>
        <li>FORMAT-SELECT: menu pilih format foto GC2145 (JPG atau BMP)</li>
        <li>stegoBmpExtract(): tidak berubah, sudah benar</li>
        <li>ISLAND-1..4: animasi Dynamic Island smooth (cubic easing)</li>
        <li>EXIF: inject APP1 EXIF ke setiap JPEG</li>
        <li>STEGO-JPEG: embed payload ke COM marker</li>
        <li>FIX-B: applyExpPreset() menjaga hmirror/vflip setelah ganti preset</li>
      </ul>
    </div>
  </div>
</section>

<!-- TROUBLESHOOT -->
<section class="section" id="troubleshoot">
  <div class="section-header">
    <span class="section-num">22</span>
    <span class="section-title">Troubleshooting</span>
  </div>
  <table>
    <thead><tr><th>Masalah</th><th>Kemungkinan Penyebab</th><th>Solusi</th></tr></thead>
    <tbody>
      <tr>
        <td>Layar blank / hitam</td>
        <td>Wiring SPI salah, pin DC/CS/RST tidak sesuai</td>
        <td>Cek <code>cfg.pin_rst = 1</code>, <code>cfg.pin_cs = 21</code>, <code>cfg.pin_dc = 14</code></td>
      </tr>
      <tr>
        <td>Kamera tidak terdeteksi</td>
        <td>Kabel flat longgar, XCLK tidak tersambung</td>
        <td>Serial monitor: cek PID di boot log. Pastikan GPIO 15 = XCLK</td>
      </tr>
      <tr>
        <td>SD card tidak terbaca</td>
        <td>Format bukan FAT32, pullup lemah, SD tidak kompatibel</td>
        <td>Format ulang FAT32 (alloc unit 16KB), cek CMD/CLK/D0</td>
      </tr>
      <tr>
        <td>PSRAM alloc failed + restart</td>
        <td>PSRAM tidak aktif di board config</td>
        <td>Set PSRAM = OPI PSRAM 8MB di Arduino IDE board settings</td>
      </tr>
      <tr>
        <td>Stego BMP tidak terbaca</td>
        <td>Foto diambil dengan firmware v5.9 lama (bug embed)</td>
        <td>Foto lama tidak kompatibel. Ambil ulang dengan v5.9-fix</td>
      </tr>
      <tr>
        <td>Face detect sangat lambat</td>
        <td>Normal — dual-model MSR01+MNP01 berat</td>
        <td>Matikan saat tidak perlu: <code>BTN_D short</code></td>
      </tr>
      <tr>
        <td>USB mode: host tidak detect</td>
        <td>Kabel bukan data cable, atau butuh reconnect</td>
        <td>Pastikan kabel USB data (bukan charge only), coba cabut-pasang USB</td>
      </tr>
      <tr>
        <td>Gallery kosong padahal ada file</td>
        <td>Nama file tidak sesuai pola atau SD tidak mount</td>
        <td>File harus berekstensi <code>.jpg</code>/<code>.bmp</code>/<code>.mjpeg</code>, SD harus <code>sdReady=true</code></td>
      </tr>
    </tbody>
  </table>
</section>

</div><!-- /content -->
</div><!-- /layout -->
</main>

<!-- FOOTER -->
<footer>
  <div>SANZXCAM v5.9-fix — ESP32-S3 Camera Firmware</div>
  <div>Built for <span>Freenove ESP32-S3-WROOM</span> + ILI9341 2.4"</div>
</footer>

<script>
  // Highlight active sidebar link on scroll
  const sections = document.querySelectorAll('section[id]');
  const sidebarLinks = document.querySelectorAll('.sidebar a');

  const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
      if (entry.isIntersecting) {
        sidebarLinks.forEach(a => a.classList.remove('active'));
        const active = document.querySelector(`.sidebar a[href="#${entry.target.id}"]`);
        if (active) active.classList.add('active');
      }
    });
  }, { rootMargin: '-30% 0px -65% 0px' });

  sections.forEach(s => observer.observe(s));
</script>
</body>
</html>
