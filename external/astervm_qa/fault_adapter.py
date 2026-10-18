"""Deliberate fault injection; never use as an execution backend."""
import json
import sys
from unicorn_adapter import run

request=json.load(sys.stdin)
result=run(request)
result['registers']['ax'] ^= 1
print(json.dumps(result))
