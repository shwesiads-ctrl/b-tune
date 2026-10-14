#!/usr/bin/env python3
"""Launch the prebuilt local preview, verifying files and server readiness."""
import argparse
import functools
from html import escape
from html.parser import HTMLParser
import http.server
import json
from pathlib import Path
import threading
import urllib.parse
import urllib.request
import uuid
import webbrowser

VERSION = '0.7.0'
REQUIRED = ('index.html', 'matale-dsp.wasm', 'matale-worklet.js',
            'vocal-tools.js', 'note-worker.js', 'pitch-editor.js')


class Assets(HTMLParser):
    def __init__(self):
        super().__init__()
        self.paths = []

    def handle_starttag(self, tag, attrs):
        if tag not in ('script', 'link'):
            return
        for key, value in attrs:
            if key not in ('src', 'href') or not value:
                continue
            url = urllib.parse.urlsplit(value)
            if not url.scheme and not url.netloc:
                self.paths.append(urllib.parse.unquote(url.path).lstrip('/'))


def missing_files(dist):
    missing = [name for name in REQUIRED if not (dist / name).is_file()]
    if 'index.html' not in missing:
        parser = Assets()
        parser.feed((dist / 'index.html').read_text(encoding='utf-8'))
        for name in parser.paths:
            path = (dist / name).resolve()
            if not (path == dist.resolve() or dist.resolve() in path.parents) or not path.is_file():
                missing.append(name)
    return missing


def locate_build(root):
    candidates = (root / 'dist', root / 'Web' / 'dist',
                  root / 'MataleTune-Browser' / 'dist',
                  root.parent / 'MataleTune-Browser' / 'dist')
    for candidate in candidates:
        if not missing_files(candidate):
            return candidate.resolve()
    raise FileNotFoundError(
        'No complete browser build found. Extract the complete repository ZIP, '
        'then run the launcher inside MataleTune-Browser. '
        'For a source-only checkout, run npm ci and npm run build in project/Web. '
        'Do not move the launcher away from its folder.')


class PreviewHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def do_GET(self):
        path = urllib.parse.urlsplit(self.path).path
        if path == '/_matale/health':
            payload = json.dumps({'app': 'Matale Tune', 'version': VERSION,
                                  'instance': self.server.instance_id}).encode()
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
            return
        if path in ('/', '/index.html') and not (Path(self.directory) / 'index.html').is_file():
            message = ('The Matale Tune folder was moved or deleted while this server '
                       'was running. Close this launcher with Ctrl+C. Extract the '
                       'complete ZIP and run its Start-Browser launcher again. '
                       'Use the new browser URL it opens, not an old tab.')
            body = ('<!doctype html><meta charset="utf-8"><title>Restart Matale Tune</title>'
                    '<body style="font:18px system-ui;max-width:700px;margin:60px auto;padding:20px">'
                    '<h1>Restart Matale Tune</h1><p>' + escape(message) + '</p></body>').encode()
            self.send_response(503)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Content-Length', str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        super().do_GET()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', type=int, default=8765, help='Preferred local port (0 chooses a free port)')
    parser.add_argument('--strict-port', action='store_true', help='Fail instead of choosing a free port if occupied')
    parser.add_argument('--no-open', action='store_true', help='Do not open a browser automatically')
    args = parser.parse_args()
    if not 0 <= args.port <= 65535:
        parser.error('Port must be between 0 and 65535.')
    try:
        dist = locate_build(Path(__file__).resolve().parent)
    except (OSError, UnicodeError, ValueError) as exc:
        parser.exit(1, str(exc) + '\n')
    handler = functools.partial(PreviewHandler, directory=str(dist))
    try:
        server = http.server.ThreadingHTTPServer(('127.0.0.1', args.port), handler)
    except OSError as exc:
        if args.strict_port or args.port == 0:
            parser.exit(1, 'Could not start the local preview: ' + str(exc) + '\n')
        print('Preferred port ' + str(args.port) + ' is unavailable. Opening this version on a free port.', flush=True)
        try:
            server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
        except OSError as fallback:
            parser.exit(1, 'Could not start the local preview: ' + str(fallback) + '\n')
    server.instance_id = uuid.uuid4().hex
    url = 'http://127.0.0.1:' + str(server.server_port)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        # Verify our own process rather than accidentally opening another app or
        # a stale server already bound to the preferred port. Ignore proxy env
        # for these loopback-only checks.
        client = urllib.request.build_opener(urllib.request.ProxyHandler({}))
        with client.open(url + '/_matale/health', timeout=5) as response:
            health = json.load(response)
        if health.get('instance') != server.instance_id:
            raise RuntimeError('The preview server identity could not be verified.')
        with client.open(url + '/', timeout=5) as response:
            if response.status != 200 or b'id="root"' not in response.read():
                raise RuntimeError('The browser page did not pass its readiness check.')
        print('Matale Tune ' + VERSION + ' browser preview: ' + url + '/', flush=True)
        print('Build folder: ' + str(dist), flush=True)
        print('Use this exact URL; old tabs may point to a previous server.', flush=True)
        print('Choose Vocal file or Mic input. Microphone monitoring starts muted.', flush=True)
        print('Audio stays here. Keep this window open; Ctrl+C stops the preview.', flush=True)
        if not args.no_open:
            webbrowser.open(url + '/')
        while thread.is_alive():
            thread.join(timeout=.5)
    except KeyboardInterrupt:
        print('\nPreview stopped.')
    except (OSError, ValueError, RuntimeError) as exc:
        parser.exit(1, 'Preview startup failed: ' + str(exc) + '\n')
    finally:
        server.shutdown()
        server.server_close()
        thread.join(timeout=5)


if __name__ == '__main__':
    main()
