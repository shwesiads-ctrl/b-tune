import assert from 'node:assert/strict';
import { chromium } from 'playwright';
import { spawn } from 'node:child_process';
import { mkdtemp, writeFile, readFile, rm, mkdir } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { analyzeVocal } from '../public/pitch-editor.js';

const web = fileURLToPath(new URL('..', import.meta.url));
const origin = 'http://127.0.0.1:5183';
const temp = await mkdtemp(join(tmpdir(), 'matale-browser-'));
const screenshotDir = process.env.MATALE_SCREENSHOTS || join(temp, 'screenshots');
await mkdir(screenshotDir, { recursive: true });
const server = spawn(process.execPath, [join(web, 'node_modules/vite/bin/vite.js'), 'preview', '--host', '127.0.0.1', '--port', '5183', '--strictPort'], { cwd: web, stdio: 'pipe' });
let logs = ''; server.stdout.on('data', b => { logs += b; }); server.stderr.on('data', b => { logs += b; });
let browser;
try {
  let ready = false;
  for (let i = 0; i < 100; i++) {
    if (server.exitCode !== null) throw new Error('Preview server exited: ' + logs);
    try { if ((await fetch(origin)).ok) { ready = true; break; } } catch {}
    await new Promise(r => setTimeout(r, 100));
  }
  assert(ready, 'Preview server did not start');
  browser = await chromium.launch({
    ...(process.env.MATALE_CHROMIUM_PATH ? { executablePath: process.env.MATALE_CHROMIUM_PATH } : {}),
    args: ['--no-sandbox', '--disable-dev-shm-usage', '--autoplay-policy=no-user-gesture-required', '--use-fake-device-for-media-stream', '--use-fake-ui-for-media-stream']
  });
  const context = await browser.newContext({ viewport: { width: 1536, height: 1050 }, permissions: ['microphone'] });
  const page = await context.newPage();
  const errors = []; page.on('pageerror', e => errors.push(e.message));
  await page.addInitScript(() => {
    const original = navigator.mediaDevices.getUserMedia.bind(navigator.mediaDevices);
    navigator.mediaDevices.getUserMedia = async constraints => {
      const stream = await original(constraints); globalThis.__mataleTracks = stream.getTracks(); return stream;
    };
  });
  await page.goto(origin);
  await page.getByRole('heading', { name: 'Matale Tune', exact: true }).waitFor();
  assert(await page.getByRole('button', { name: 'Set Audio On Auto', exact: true }).isDisabled());
  assert(await page.getByRole('button', { name: 'Stop', exact: true }).isDisabled());
  await page.screenshot({ path: join(screenshotDir, 'Browser-Home.png'), fullPage: true });

  // A real PCM WAV exercises decodeAudioData -> AudioWorklet -> shared WASM.
  const rate = 48000, frames = rate * 5, wav = Buffer.alloc(44 + frames * 2);
  wav.write('RIFF'); wav.writeUInt32LE(wav.length - 8, 4); wav.write('WAVEfmt ', 8);
  wav.writeUInt32LE(16, 16); wav.writeUInt16LE(1, 20); wav.writeUInt16LE(1, 22);
  wav.writeUInt32LE(rate, 24); wav.writeUInt32LE(rate * 2, 28); wav.writeUInt16LE(2, 32); wav.writeUInt16LE(16, 34);
  wav.write('data', 36); wav.writeUInt32LE(frames * 2, 40);
  for (let n = 0; n < frames; n++) wav.writeInt16LE(Math.round(.2 * 32767 * Math.sin(2 * Math.PI * 445 * n / rate)), 44 + n * 2);
  const fixture = join(temp, 'vocal-fixture.wav'); await writeFile(fixture, wav);
  await page.locator('input[accept="audio/*"]').setInputFiles(fixture);
  await page.waitForFunction(() => document.querySelector('footer')?.textContent.includes('Vocal file looping'));
  await page.waitForFunction(() => /44[0-9]/.test(document.querySelector('.note-ring small')?.textContent || ''));
  assert.equal(await page.locator('.note-ring strong').textContent(), 'A4');
  assert(Number.parseFloat(await page.locator('.output-value').textContent()) > -60);
  console.log('PASS: real browser file decoding, live WASM pitch and output telemetry');

  const correction = page.getByRole('spinbutton', { name: 'Pitch', exact: true });
  await correction.focus(); await correction.press('ArrowLeft');
  assert.equal(await correction.getAttribute('aria-valuenow'), '99');
  await page.locator('.selectors select').nth(0).selectOption('9');
  await page.locator('.selectors select').nth(1).selectOption('1');
  await page.getByRole('button', { name: 'Set Audio On Auto', exact: true }).click();
  await page.waitForFunction(() => document.querySelector('footer')?.textContent.includes('Auto applied'), undefined, { timeout: 15000 });
  assert.equal(await page.locator('.selectors select').nth(0).inputValue(), '9');
  assert.equal(await page.locator('.selectors select').nth(1).inputValue(), '1');
  assert.equal(await page.getByRole('spinbutton', { name: 'Reverb', exact: true }).getAttribute('aria-valuenow'), '0.1');
  await page.screenshot({ path: join(screenshotDir, 'Browser-Live.png'), fullPage: true });
  console.log('PASS: browser parameter interaction and Auto result with key/scale preserved');
  await page.getByRole('button',{name:'Pitch',exact:true}).click();
  const humanize=page.getByRole('spinbutton',{name:'Humanize (%)',exact:true});await humanize.press('End');
  await page.getByRole('button',{name:'Home',exact:true}).click();
  await page.getByRole('checkbox',{name:'Pitch retune',exact:true}).uncheck();
  await page.getByRole('button',{name:'Set Audio On Auto',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('Auto applied'));
  await page.getByRole('button',{name:'Pitch',exact:true}).click();assert.equal(await humanize.getAttribute('aria-valuenow'),'100','Excluded Auto group must remain unchanged');
  await page.getByRole('button',{name:'Home',exact:true}).click();
  const reverb=page.getByRole('spinbutton',{name:'Reverb',exact:true});await reverb.press('Home');
  const dynamicsBefore=await page.getByRole('spinbutton',{name:'Dynamics',exact:true}).getAttribute('aria-valuenow');
  await page.getByRole('button',{name:'Auto reverb',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('Auto applied'));
  assert.equal(await reverb.getAttribute('aria-valuenow'),'0.1');assert.equal(await page.getByRole('spinbutton',{name:'Dynamics',exact:true}).getAttribute('aria-valuenow'),dynamicsBefore);
  console.log('PASS: Auto inclusion switches and per-stage analysis scope');


  for (const name of ['Pitch', 'Dynamics', 'EQ', 'Effects']) {
    await page.getByRole('button', { name, exact: true }).click();
    assert(await page.getByRole('button', { name, exact: true }).getAttribute('aria-pressed') === 'true');
    assert(await page.locator('.rack [role=spinbutton]').count() > 0);
  }
  await page.getByRole('button', { name: 'EQ', exact: true }).click();
  const eq = page.getByRole('spinbutton', { name: 'Band 3 dB', exact: true });
  await eq.focus(); await eq.press('End'); assert.equal(await eq.getAttribute('aria-valuenow'), '12');
  await page.screenshot({ path: join(screenshotDir, 'Browser-EQ.png'), fullPage: true });
  await page.getByRole('button', { name: 'Home', exact: true }).click();
  await page.getByRole('button', { name: 'Stop', exact: true }).click();
  await page.waitForFunction(() => document.querySelector('footer')?.textContent.includes('Stopped.'));
  assert.equal(await page.locator('.note-ring strong').textContent(), '—');
  console.log('PASS: all manual pages, EQ adjustment and file Stop');

  await page.getByRole('button', { name: 'Mic input', exact: true }).click();
  await page.waitForFunction(() => document.querySelector('footer')?.textContent.includes('Microphone live'));
  const monitor = page.getByRole('checkbox', { name: 'Monitor · headphones', exact: true });
  assert.equal(await monitor.isChecked(), false); await monitor.check(); assert(await monitor.isChecked());
  await page.getByRole('button', { name: 'Stop', exact: true }).click();
  assert(await page.evaluate(() => globalThis.__mataleTracks.every(t => t.readyState === 'ended')));
  assert(await monitor.isDisabled());
  console.log('PASS: browser synthetic microphone, muted startup, monitoring toggle and track cleanup');

  // No hidden range inputs: every continuous control is a genuine knob.
  assert.equal(await page.locator('input[type=range]').count(), 0);
  assert.equal(await page.getByRole('slider').count(), 0);
  const pitch = page.getByRole('spinbutton', {name:'Pitch',exact:true});
  await pitch.focus(); await pitch.press('Home'); assert.equal(await pitch.getAttribute('aria-valuenow'),'0');
  const box=await pitch.boundingBox(); await page.mouse.move(box.x+box.width/2,box.y+box.height/2);await page.mouse.down();await page.mouse.move(box.x+box.width/2,box.y+box.height/2-90,{steps:10});await page.mouse.up();
  assert(Number(await pitch.getAttribute('aria-valuenow'))>=49,'Pointer drag must turn the actual control');
  await pitch.dblclick();assert.equal(await pitch.getAttribute('aria-valuenow'),'100');
  await page.getByRole('button',{name:'Save',exact:true}).click();
  await page.getByRole('textbox',{name:'Preset name',exact:true}).fill('Regression Vocal');
  const downloading=page.waitForEvent('download');await page.getByRole('button',{name:'Save preset',exact:true}).click();
  const download=await downloading;const preset=JSON.parse(await readFile(await download.path(),'utf8'));
  assert.equal(preset.name,'Regression Vocal');assert.equal(preset.format,'matale-tune');
  assert(await page.evaluate(()=>JSON.parse(localStorage.getItem('matale-presets-v1')).some(p=>p.name==='Regression Vocal')));
  preset.name='Imported Vocal';preset.params[7]=62;preset.extras.deess=71;
  const importFile=page.locator('input[accept="application/json,.json"]');
  await importFile.setInputFiles({name:'vocal.json',mimeType:'application/json',buffer:Buffer.from(JSON.stringify(preset))});
  await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('Imported preset: Imported Vocal'));
  assert.equal(await pitch.getAttribute('aria-valuenow'),'62');
  assert.equal(await page.getByRole('spinbutton',{name:'De-Esser',exact:true}).getAttribute('aria-valuenow'),'71');
  await page.getByRole('button',{name:'B',exact:true}).click();assert.equal(await pitch.getAttribute('aria-valuenow'),'100');
  await pitch.focus();await pitch.press('Home');
  await page.getByRole('button',{name:'A',exact:true}).click();assert.equal(await pitch.getAttribute('aria-valuenow'),'62');
  await page.getByRole('button',{name:'B',exact:true}).click();assert.equal(await pitch.getAttribute('aria-valuenow'),'0');
  await page.getByRole('button',{name:'A',exact:true}).click();
  const deessSwitch=page.getByRole('switch',{name:'De-Esser stage',exact:true});await deessSwitch.click();assert.equal(await deessSwitch.getAttribute('aria-checked'),'false');
  assert.equal(await page.getByRole('spinbutton',{name:'De-Esser',exact:true}).getAttribute('aria-valuenow'),'71');await deessSwitch.click();
  preset.params[7]=999;await importFile.setInputFiles({name:'invalid.json',mimeType:'application/json',buffer:Buffer.from(JSON.stringify(preset))});
  await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('out of range'));
  assert.equal(await pitch.getAttribute('aria-valuenow'),'62','Invalid import must not replace current settings');
  await page.getByRole('button',{name:'Settings',exact:true}).click();
  await page.getByRole('combobox',{name:'Sample rate',exact:true}).selectOption('48000');
  await page.getByRole('combobox',{name:'Latency preference',exact:true}).selectOption('balanced');
  await page.getByRole('combobox',{name:'Analysis display',exact:true}).selectOption('waveform');
  await page.getByRole('spinbutton',{name:'Monitor volume',exact:true}).press('Home');
  await page.getByRole('button',{name:'Mic input',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('Microphone live'));
  assert(await page.locator('.wave-label').textContent().then(s=>s.includes('48 kHz')));
  await page.getByRole('button',{name:'Detect Key',exact:true}).click();await page.getByRole('button',{name:'Finish key capture',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('.key-results')?.textContent.includes('Not enough melodic evidence'));
  console.log('PASS: live key capture finishes and rejects insufficient melodic evidence');
  await page.getByRole('button',{name:'Stop',exact:true}).click();
  await page.getByRole('button',{name:'Home',exact:true}).click();
  console.log('PASS: pointer/keyboard/reset knobs, zero sliders, saved/exported/imported presets, invalid import protection, A/B snapshots, stage toggles and applied audio settings');

  // Realtime controls update an active graph; note editing/key detection run in workers.
  const melodyNotes=[[60,1.5],[64,.9],[67,1.2],[62,.4],[65,.4],[69,.3],[71,.3],[60,1.2]];
  const totalFrames=Math.round(melodyNotes.reduce((n,[,duration])=>n+duration,0)*rate),melody=Buffer.alloc(44+totalFrames*2);
  wav.copy(melody,0,0,44);melody.writeUInt32LE(melody.length-8,4);melody.writeUInt32LE(totalFrames*2,40);
  let offset=0;for(const [midi,duration] of melodyNotes){const count=Math.round(duration*rate),hz=440*2**((midi-69)/12);for(let i=0;i<count;i++)melody.writeInt16LE(Math.round(.15*32767*Math.sin(2*Math.PI*hz*i/rate)),44+(offset+i)*2);offset+=count;}
  await page.locator('input[accept="audio/*"]').setInputFiles({name:'C-major-vocal.wav',mimeType:'audio/wav',buffer:melody});
  await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('Vocal file looping'));
  const inputGain=page.getByRole('spinbutton',{name:'Input gain',exact:true});await inputGain.press('Home');
  await page.waitForFunction(()=>Number.parseFloat(document.querySelector('.input-meter strong')?.textContent)<-35);
  await inputGain.dblclick();await page.waitForFunction(()=>Number.parseFloat(document.querySelector('.input-meter strong')?.textContent)>-25);
  const livePitch=page.getByRole('switch',{name:'Realtime Auto Pitch',exact:true});await livePitch.click();assert.equal(await livePitch.getAttribute('aria-checked'),'false');await livePitch.click();assert.equal(await livePitch.getAttribute('aria-checked'),'true');
  await page.getByRole('button',{name:'Detect Key',exact:true}).click();
  await page.getByRole('button',{name:'Apply C Major',exact:true}).waitFor();await page.getByRole('button',{name:'Apply C Major',exact:true}).click();
  assert.equal(await page.locator('.selectors select').nth(0).inputValue(),'0');assert.equal(await page.locator('.selectors select').nth(1).inputValue(),'1');
  await page.getByRole('button',{name:'Note Editor',exact:true}).click();
  await page.locator('.vocal-note').first().waitFor(); // Editor analyses automatically on open.
  const timeline=page.getByRole('progressbar',{name:'Vocal progress',exact:true});
  assert(await timeline.evaluate(el=>el.getBoundingClientRect().width>=el.closest('.vocal-progress').getBoundingClientRect().width-50),'Playback progress must span the panel rather than inherit the 130px global cap');
  await page.waitForFunction(()=>Number(document.querySelector('progress[aria-label="Vocal progress"]').value)>0);
  await page.getByRole('button',{name:'Pause vocal',exact:true}).click();
  const seek=page.getByRole('button',{name:'Seek vocal timeline',exact:true}),seekBox=await seek.boundingBox();
  await seek.click({position:{x:seekBox.width*.5,y:seekBox.height/2}});
  const midpoint=Number(await timeline.getAttribute('max'))/2;
  assert.ok(Math.abs(Number(await timeline.getAttribute('value'))-midpoint)<.05,'Click seeks to the middle while paused');
  await seek.press('Home');assert.equal(Number(await timeline.getAttribute('value')),0);
  await seek.press('ArrowRight');assert.equal(Number(await timeline.getAttribute('value')),5);
  await seek.press('ArrowLeft');assert.equal(Number(await timeline.getAttribute('value')),0);
  const paused=await timeline.getAttribute('value');await page.waitForTimeout(350);assert.equal(await timeline.getAttribute('value'),paused,'Pause freezes the playback timeline');
  await page.getByRole('button',{name:'Resume vocal',exact:true}).click();
  await page.getByRole('button',{name:'Restart vocal',exact:true}).click();
  await page.waitForTimeout(150);assert.ok(Number(await timeline.getAttribute('value'))<1,'Restart returns to the beginning');
  assert(await page.locator('.vocal-note').count()>=6);
  await page.locator('.vocal-note').first().click();const selectedPitch=page.getByRole('spinbutton',{name:'Selected note pitch',exact:true});
  const originalNote=Number(await selectedPitch.getAttribute('aria-valuenow'));await selectedPitch.press('ArrowUp');assert.equal(Number(await selectedPitch.getAttribute('aria-valuenow')),originalNote+1);
  await page.getByRole('button',{name:'Undo',exact:true}).click();assert.equal(Number(await selectedPitch.getAttribute('aria-valuenow')),originalNote);
  await page.getByRole('button',{name:'Redo',exact:true}).click();assert.equal(Number(await selectedPitch.getAttribute('aria-valuenow')),originalNote+1);
  const secondNote=page.locator('.vocal-note').nth(1),noteBox=await secondNote.boundingBox();await secondNote.scrollIntoViewIfNeeded();
  await page.mouse.move(noteBox.x+noteBox.width/2,noteBox.y+noteBox.height/2);await page.mouse.down();await page.mouse.move(noteBox.x+noteBox.width/2,noteBox.y-noteBox.height/2,{steps:4});await page.mouse.up();
  await page.getByRole('button',{name:'Preview edited',exact:true}).click(); // Must render current edits without a separate Render click.
  await page.waitForFunction(()=>!Array.from(document.querySelectorAll('button')).find(b=>b.textContent==='Export WAV')?.disabled);
  await page.screenshot({path:join(screenshotDir,'Browser-Note-Editor.png'),fullPage:true});
  const wavDownload=page.waitForEvent('download');await page.getByRole('button',{name:'Export WAV',exact:true}).click();
  const wavFile=await wavDownload,exported=await readFile(await wavFile.path()),renderRate=exported.readUInt32LE(24),channels=exported.readUInt16LE(22),pcm=new Float32Array((exported.length-44)/(2*channels));
  for(let i=0;i<pcm.length;i++)pcm[i]=exported.readInt16LE(44+i*2*channels)/32768;
  const measured=analyzeVocal(pcm.slice(Math.round(renderRate*.4),Math.round(renderRate*1.2)),renderRate);
  assert(measured.notes.some(n=>Math.abs(n.midi-(originalNote+1))<.3),'Downloaded edited WAV did not contain the changed note pitch');
  assert(Math.abs(pcm.length/renderRate-totalFrames/rate)<.001,'Edited WAV changed clip duration');
  await page.getByRole('button',{name:'Preview edited',exact:true}).click();await page.waitForFunction(()=>document.querySelector('footer')?.textContent.includes('note preview looping'));
  assert(await livePitch.isDisabled(),'Note preview must bypass realtime correction to preserve deliberate edits');
  await page.getByRole('button',{name:'Stop',exact:true}).click();await page.getByRole('button',{name:'Home',exact:true}).click();
  await page.setViewportSize({width:2880,height:1600});
  assert(await page.evaluate(()=>document.querySelector('.workspace').getBoundingClientRect().right>innerWidth-40),'Workspace leaves a large blank area on wide screens');
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
  await page.getByRole('button',{name:'Settings',exact:true}).click();await page.getByRole('combobox',{name:'Interface size',exact:true}).selectOption('1.15');
  assert(await page.evaluate(()=>Number.parseFloat(getComputedStyle(document.querySelector('main')).zoom)>1));
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),'Large interface causes horizontal overflow');
  await page.getByRole('combobox',{name:'Interface size',exact:true}).selectOption('1');await page.getByRole('button',{name:'Home',exact:true}).click();
  console.log('PASS: live input gain, Auto Pitch toggle, file key detection/apply, note editing/undo/redo, real edited WAV pitch/duration, preview bypass and wide layout/interface sizing');

  await page.setViewportSize({ width: 390, height: 844 });
  assert(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
  await page.screenshot({ path: join(screenshotDir, 'Browser-Mobile.png'), fullPage: true });
  assert.deepEqual(errors, []);
  console.log('PASS: narrow viewport without horizontal overflow; no uncaught browser errors');
  console.log('Screenshots:', screenshotDir);
} finally {
  await browser?.close(); server.kill('SIGTERM');
  if (server.exitCode === null) await new Promise(resolve => server.once('exit', resolve));
  await rm(temp, { recursive: true, force: true });
}
