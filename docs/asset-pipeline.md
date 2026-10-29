# Asset pipeline

The saved scripts were actually executed through the live Blender MCP Python tool.
Blender 5.2.3 LTS was verified. `art/blender/vault.blend` and `thief.blend` are editable;
`assets/exchange/vault.glb` and `thief.glb` are portable exchange resources.

1. Run `tools/blender/make_textures.py` with Python and Pillow to regenerate seeded PNGs.
2. In Blender's Python environment add `tools/blender` to sys.path, then execute
   `build_room.py` and `build_thief.py` using their actual absolute `__file__` paths.
3. The builders call `export_scene.py` / `export_character.py` for native runtime data.
4. Execute `validate_character.py` to sample poses, ground bounds and normalized weights.
5. Execute `export_exchange.py` to save the two isolated GLBs. It uses both selection
   and `use_active_scene=True`; this is essential when several scenes are open.

Example Blender Python invocation (replace the project path):

```python
import sys
from pathlib import Path
script = Path('/absolute/project/tools/blender/build_room.py')
sys.path.insert(0, str(script.parent))
exec(compile(script.read_text(encoding='utf-8'), str(script), 'exec'),
     {'__name__': '__main__', '__file__': str(script)})
```

The scripts recreate their named project collections/scenes; preserve unrelated work
before deliberately rebuilding these assets. Blender API enum choices are inspected
against the installed version. Do not run the builders in another existing game's file.

The thief has 21,782 triangles, 20 bones, normalized weights with at most four available
influences per vertex, and Idle, Walk, CrouchIdle, CrouchWalk and Interact actions. Actual
weights use at most two influences. Pose validation sampled the lowest point at 0.004 m,
standing maximum 1.8909 m, crouch maximum 1.2755 m. Runtime collision heights are 1.90 m
and 1.29 m. Camera eyes are 1.67 m and 1.13 m. The 1.42 m final beam blocks standing and
permits crouching. Walk stride is 1.2 m and crouch stride 0.75 m; phase follows movement.

The safe has independent wheel, sliding bolts and hinged door nodes. Spot fixtures
parent the visible heads and lights. GLB animation and skin counts are recorded in
`evidence/glb-validation.json`; runtime pose bounds are in
`evidence/character-pose-bounds.json`. Custom Blinn-Phong material parameters are explicit;
no claim is made that Principled BSDF is reproduced exactly.
