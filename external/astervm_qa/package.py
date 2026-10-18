import hashlib
from pathlib import Path
import zipfile

root=Path(__file__).resolve().parent
files=sorted(p for p in root.rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='SHA256SUMS')
(root/'SHA256SUMS').write_text(''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.relative_to(root).as_posix()+'\n' for p in files))
target=root.parent/'astervm_qa_v0.1.zip'
with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
    for p in files+[root/'SHA256SUMS']:z.write(p,'astervm_qa/'+p.relative_to(root).as_posix())
with zipfile.ZipFile(target) as z:
    assert z.testzip() is None
    assert 'astervm_qa/vectors.json' in z.namelist()
print(target.name, target.stat().st_size, 'bytes')
print('SHA256',hashlib.sha256(target.read_bytes()).hexdigest())
