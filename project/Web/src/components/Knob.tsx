import { useRef } from 'react';
import type { KeyboardEvent, PointerEvent } from 'react';

type Props = { label: string; min: number; max: number; step: number; value: number; initial?: number; text?: string; disabled?: boolean; onChange: (value: number) => void };
export default function Knob({ label, min, max, step, value, initial = min, text, disabled = false, onChange }: Props) {
  const drag = useRef<{ y: number; value: number } | null>(null);
  const quantize = (v: number) => Number(Math.max(min, Math.min(max, min + Math.round((v - min) / step) * step)).toFixed(6));
  function down(e: PointerEvent<HTMLButtonElement>) {
    if (disabled || e.button !== 0) return;
    e.currentTarget.focus(); e.currentTarget.setPointerCapture(e.pointerId);
    drag.current = { y: e.clientY, value };
  }
  function key(e: KeyboardEvent<HTMLButtonElement>) {
    let next = value;
    if (e.key === 'Home') next = min;
    else if (e.key === 'End') next = max;
    else if (['ArrowUp', 'ArrowRight', 'PageUp'].includes(e.key)) next += step * (e.shiftKey || e.key === 'PageUp' ? 10 : 1);
    else if (['ArrowDown', 'ArrowLeft', 'PageDown'].includes(e.key)) next -= step * (e.shiftKey || e.key === 'PageDown' ? 10 : 1);
    else return;
    e.preventDefault(); onChange(quantize(next));
  }
  return <div className={'rotary-control ' + (disabled ? 'disabled' : '')}>
    <span className="knob-label">{label}</span>
    <button type="button" className="rotary" role="spinbutton" aria-label={label} aria-valuemin={min} aria-valuemax={max} aria-valuenow={value} aria-valuetext={text ?? String(value)} disabled={disabled}
      title="Drag up/down to turn. Arrow keys adjust; Shift is coarse. Double-click resets."
      onKeyDown={key} onPointerDown={down} onPointerMove={e => {
        if (!drag.current || disabled) return;
        const sensitivity = (max - min) / (e.shiftKey ? 1000 : 180);
        onChange(quantize(drag.current.value + (drag.current.y - e.clientY) * sensitivity));
      }} onPointerUp={e => { drag.current = null; if (e.currentTarget.hasPointerCapture(e.pointerId)) e.currentTarget.releasePointerCapture(e.pointerId); }}
      onPointerCancel={() => { drag.current = null; }} onLostPointerCapture={() => { drag.current = null; }} onDoubleClick={() => onChange(quantize(initial))}>
      <svg viewBox="0 0 100 100" aria-hidden="true"><path className="rotary-track" d="M 20.3 79.7 A 42 42 0 1 1 79.7 79.7" /><path className="rotary-fill" d="M 20.3 79.7 A 42 42 0 1 1 79.7 79.7" pathLength="100" strokeDasharray={`${100 * (value - min) / (max - min)} 100`} /></svg>
      <span className="rotary-disc"><i style={{ transform: `rotate(${-135 + 270 * (value - min) / (max - min)}deg)` }} /></span>
    </button><output>{text ?? value}</output>
  </div>;
}
