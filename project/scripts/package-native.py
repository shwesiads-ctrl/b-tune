#!/usr/bin/env python3
"""Package a verified Windows x64 VST3 bundle, not source disguised as a plugin."""
import argparse,hashlib,shutil,struct,subprocess,tempfile,zipfile,sys
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('--build-dir',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
args=parser.parse_args()
project=Path(__file__).resolve().parent.parent
repo=project.parent
bundle=args.build_dir/'MataleTune_artefacts/Release/VST3/Matale Tune.vst3'
module=bundle/'Contents/x86_64-win/Matale Tune.vst3'
if not module.is_file():raise SystemExit('Missing Windows VST3 module: '+str(module))
data=module.read_bytes()
if data[:2]!=b'MZ' or len(data)<64:raise SystemExit('Not a Windows PE module')
offset=struct.unpack_from('<I',data,0x3c)[0]
if data[offset:offset+4]!=b'PE\0\0' or struct.unpack_from('<H',data,offset+4)[0]!=0x8664:raise SystemExit('Expected an x64 Windows PE module')
subprocess.run([sys.executable,str(project/'scripts/sync-web-source.py')],check=True)
try:revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
except (OSError,subprocess.CalledProcessError):revision='local build'
with tempfile.TemporaryDirectory(prefix='matale-package-') as tmp:
    root=Path(tmp)/'MataleTune-Windows-x64-VST3'
    root.mkdir()
    shutil.copytree(bundle,root/bundle.name)
    for source,target in [(repo/'LICENSE','LICENSE-project-MIT.txt'),(repo/'THIRD_PARTY.md','THIRD-PARTY-NOTICES.md'),(project/'docs/DAW-TESTING.md','READ-ME-FIRST.md'),(project/'Web/public/MataleTune_Source.zip','MataleTune_Source.zip')]:shutil.copy2(source,root/target)
    juce=args.build_dir/'_deps/juce-src/LICENSE.md'
    if juce.is_file():shutil.copy2(juce,root/'JUCE-LICENSE.md')
    (root/'BUILD-INFO.txt').write_text('Matale Tune 0.9.1 DAW preview\nWindows x64 / VST3\nSource: '+revision+'\nOriginal code: MIT. JUCE and other dependencies retain their licenses.\n',encoding='utf-8')
    args.output.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(root.rglob('*')):
            if path.is_file():archive.write(path,path.relative_to(root.parent))
with zipfile.ZipFile(args.output) as archive:
    if archive.testzip() is not None:raise SystemExit('ZIP CRC validation failed')
digest=hashlib.sha256(args.output.read_bytes()).hexdigest()
args.output.with_suffix('.zip.sha256').write_text(digest+'  '+args.output.name+'\n',encoding='ascii')
print(str(args.output)+' SHA256 '+digest)
