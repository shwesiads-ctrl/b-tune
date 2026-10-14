#!/usr/bin/env python3
"""Run the prebuilt browser preview locally. Python 3, no extra packages."""
import argparse
import functools
import http.server
from pathlib import Path
import webbrowser


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=8765, help='Local server port (0 selects an available port)')
    parser.add_argument('--no-open', action='store_true', help='Do not automatically open the browser')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    dist = root / 'Web' / 'dist' if (root / 'Web' / 'dist').is_dir() else root / 'dist'
    for name in ('index.html', 'matale-dsp.wasm', 'matale-worklet.js', 'vocal-tools.js', 'note-worker.js', 'pitch-editor.js'):
        if not (dist / name).is_file():
            parser.error('Missing browser build: ' + str(dist / name))
    handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=str(dist))
    try:
        server = http.server.ThreadingHTTPServer(('127.0.0.1', args.port), handler)
    except OSError as exc:
        parser.exit(1, 'Could not start the local preview: ' + str(exc) + '\nTry --port 0 to select a free port.\n')
    url = 'http://127.0.0.1:' + str(server.server_port)
    print('Matale Tune browser preview: ' + url, flush=True)
    print('Choose Vocal file, or Mic input. Microphone monitoring starts muted.', flush=True)
    print('Audio stays on this computer. Press Ctrl+C here to stop the server.', flush=True)
    if not args.no_open:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print('\nPreview stopped.')
    finally:
        server.server_close()


if __name__ == '__main__':
    main()
