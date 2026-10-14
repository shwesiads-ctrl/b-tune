"""Functional tests for preview startup, port collisions and moved folders."""
import functools
import http.server
import json
from pathlib import Path
import queue
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import unittest
import urllib.error
import urllib.request

LAUNCHER = Path(__file__).resolve().parents[1] / 'Start-Browser.py'
CLIENT = urllib.request.build_opener(urllib.request.ProxyHandler({}))


class LauncherTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='matale-launcher-')
        self.root = Path(self.temp.name)
        self.processes = []
        self.servers = []

    def tearDown(self):
        for p in self.processes:
            if p.poll() is None:
                p.send_signal(signal.SIGINT)
                try:
                    p.communicate(timeout=5)
                except subprocess.TimeoutExpired:
                    p.kill()
                    p.communicate(timeout=5)
        for s, thread in self.servers:
            s.shutdown()
            s.server_close()
            thread.join(timeout=5)
        self.temp.cleanup()

    def build(self, folder):
        folder.mkdir(parents=True)
        shutil.copy2(LAUNCHER, folder / 'Start-Browser.py')
        dist = folder / 'dist'
        (dist / 'assets').mkdir(parents=True)
        (dist / 'index.html').write_text('<html><head><script src="/assets/app.js"></script></head><body><div id="root">Matale test build</div></body></html>')
        for name in ('matale-dsp.wasm', 'matale-worklet.js', 'vocal-tools.js', 'note-worker.js', 'pitch-editor.js', 'assets/app.js'):
            (dist / name).write_bytes(b'fixture')
        return dist

    def launch(self, script, *args):
        p = subprocess.Popen([sys.executable, str(script), '--no-open', *args], cwd=self.root,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.processes.append(p)
        lines = queue.Queue()
        def consume():
            for line in p.stdout:
                lines.put(line)
        threading.Thread(target=consume, daemon=True).start()
        for _ in range(30):
            try:
                line = lines.get(timeout=.3)
            except queue.Empty:
                if p.poll() is not None:
                    self.fail('Launcher exited: ' + p.stderr.read())
                continue
            match = re.search(r'browser preview: (http://127\.0\.0\.1:\d+/)', line)
            if match:
                return match[1]
        self.fail('Launcher did not report a ready URL')

    def old_server(self):
        # Match a stale process whose directory was deleted after an upgrade.
        handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=str(self.root / 'deleted-old-build'))
        server = http.server.ThreadingHTTPServer(('127.0.0.1', 0), handler)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        self.servers.append((server, thread))
        return server.server_port

    def test_ready_from_other_working_directory_and_unicode_folder(self):
        folder = self.root / 'Matale Tune မြန်မာ' / 'MataleTune-Browser'
        self.build(folder)
        url = self.launch(folder / 'Start-Browser.py', '--port', '0')
        with CLIENT.open(url) as response:
            self.assertIn(b'Matale test build', response.read())
            self.assertEqual(response.headers['Cache-Control'], 'no-store')
        with CLIENT.open(url + 'assets/app.js') as response:
            self.assertEqual(response.read(), b'fixture')
        with CLIENT.open(url + '_matale/health') as response:
            self.assertEqual(json.load(response)['app'], 'Matale Tune')

    def test_stale_404_server_does_not_capture_new_launch(self):
        folder = self.root / 'new' / 'MataleTune-Browser'
        self.build(folder)
        port = self.old_server()
        with self.assertRaises(urllib.error.HTTPError) as error:
            CLIENT.open('http://127.0.0.1:' + str(port) + '/')
        self.assertEqual(error.exception.code, 404)
        url = self.launch(folder / 'Start-Browser.py', '--port', str(port))
        self.assertNotIn(':' + str(port) + '/', url)
        with CLIENT.open(url) as response:
            self.assertEqual(response.status, 200)

    def test_moved_build_shows_restart_instructions(self):
        folder = self.root / 'MataleTune-Browser'
        dist = self.build(folder)
        url = self.launch(folder / 'Start-Browser.py', '--port', '0')
        dist.rename(folder / 'moved-dist')
        with self.assertRaises(urllib.error.HTTPError) as error:
            CLIENT.open(url)
        self.assertEqual(error.exception.code, 503)
        self.assertIn(b'Restart Matale Tune', error.exception.read())

    def test_partial_build_fails_before_browser_opens(self):
        folder = self.root / 'partial' / 'MataleTune-Browser'
        dist = self.build(folder)
        (dist / 'assets/app.js').unlink()
        result = subprocess.run([sys.executable, str(folder / 'Start-Browser.py'), '--no-open', '--port', '0'], capture_output=True, text=True, timeout=5)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('No complete browser build found', result.stderr)

    def test_source_launcher_finds_ready_sibling_package(self):
        self.build(self.root / 'MataleTune-Browser')
        source = self.root / 'project'
        source.mkdir()
        shutil.copy2(LAUNCHER, source / 'Start-Browser.py')
        url = self.launch(source / 'Start-Browser.py', '--port', '0')
        with CLIENT.open(url) as response:
            self.assertEqual(response.status, 200)

    def test_strict_port_reports_conflict(self):
        folder = self.root / 'MataleTune-Browser'
        self.build(folder)
        port = self.old_server()
        result = subprocess.run([sys.executable, str(folder / 'Start-Browser.py'), '--no-open', '--port', str(port), '--strict-port'], capture_output=True, text=True, timeout=5)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Could not start the local preview', result.stderr)


if __name__ == '__main__':
    unittest.main(verbosity=2)
