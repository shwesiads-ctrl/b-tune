import assert from 'node:assert/strict';
import React from 'react';
import { create, act } from 'react-test-renderer';
import { createServer } from 'vite';
globalThis.IS_REACT_ACT_ENVIRONMENT = true;
globalThis.requestAnimationFrame = () => 1;
globalThis.cancelAnimationFrame = () => {};
const deferred = () => { let resolve; const promise = new Promise(r => { resolve = r; }); return { promise, resolve }; };
let closeGate, requests, contexts, nodes, tracks;
class Node { connect() {} disconnect() {} }
globalThis.AudioContext = class {
  constructor() { this.state = 'running'; this.sampleRate = 48000; this.destination = {}; this.audioWorklet = { addModule: async () => {} }; contexts.push(this); }
  async resume() {}
  async close() { this.state = 'closed'; if (closeGate) await closeGate.promise; }
  createAnalyser() { return Object.assign(new Node(), { getFloatTimeDomainData() {} }); }
  createGain() { return Object.assign(new Node(), { gain: { value: 0, setTargetAtTime() {} } }); }
  createMediaStreamSource() { assert.notEqual(this.state, 'closed'); return new Node(); }
};
globalThis.AudioWorkletNode = class extends Node {
  constructor() { super(); this.port = { postMessage() {}, onmessage: null }; nodes.push(this); }
};
Object.defineProperty(globalThis, 'navigator', { configurable: true, value: { mediaDevices: {
  getUserMedia() { const d = deferred(); requests.push(d); return d.promise; }
} } });
globalThis.fetch = async () => ({ ok: true, arrayBuffer: async () => new ArrayBuffer(0) });
const originalCompile = WebAssembly.compile;
WebAssembly.compile = async () => ({});
const server = await createServer({ server: { middlewareMode: true }, appType: 'custom' });
try {
  const { default: Workbench } = await server.ssrLoadModule('/src/components/PluginUiWorkbench.tsx');
  async function setup() {
    closeGate = null; requests = []; contexts = []; nodes = []; tracks = [];
    let view;
    await act(async () => { view = create(React.createElement(Workbench)); });
    const button = text => view.root.findAllByType('button').find(b => b.children.join('') === text);
    const click = async text => { await act(async () => { button(text).props.onClick(); }); };
    const mic = async index => {
      const track = { stopped: false, stop() { this.stopped = true; } }; tracks.push(track);
      await act(async () => { requests[index].resolve({ getTracks: () => [track] }); }); return track;
    };
    return { view, button, click, mic };
  }
  {
    const { view, click, mic } = await setup();
    await click('Mic input'); assert.equal(requests.length, 1);
    await click('Stop'); const track = await mic(0);
    assert(track.stopped); assert(contexts.every(c => c.state === 'closed'));
    await act(async () => view.unmount());
    console.log('PASS: Stop releases microphone permission granted after cancellation');
  }
  {
    const { view, click, mic, button } = await setup();
    await click('Mic input'); await mic(0); closeGate = deferred();
    await click('Mic input');
    assert.equal(button('Stop').props.disabled, false, 'Stop must remain available during restart');
    await click('Stop');
    await act(async () => closeGate.resolve());
    assert.equal(contexts.length, 1, 'cancelled restart must not create a context');
    assert.equal(requests.length, 1, 'cancelled restart must not request a microphone');
    assert.equal(button('Stop').props.disabled, true);
    await act(async () => view.unmount());
    console.log('PASS: Stop cancels restart while the previous context closes');
  }
  {
    const { view, click, mic, button } = await setup();
    await click('Mic input'); await mic(0);
    await act(async () => nodes[0].onprocessorerror());
    assert(tracks[0].stopped); assert.equal(contexts[0].state, 'closed');
    assert.equal(button('Stop').props.disabled, true);
    assert(view.root.findByType('footer').children.some(c => typeof c === 'string' && c.includes('Audio processing failed')));
    await act(async () => view.unmount());
    console.log('PASS: processor failure releases microphone and reports error');
  }
} finally { WebAssembly.compile = originalCompile; await server.close(); }
