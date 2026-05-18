To test:

```bash
cd python
python3 -m venv .venv # Only needs to be done once.
source .venv/bin/activate
pip install -e .[dev] -v
python tests/test_smoke.py

```