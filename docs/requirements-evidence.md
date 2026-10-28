# Requirements and evidence

All statuses describe actual verified work. A working scaffold does not close final artistic, documentation or delivery requirements.

| Criterion | Requirement / points | Status | Evidence |
|---|---|---|---|
| R1 | C++ + OpenGL 3.3+ core, собственные GLSL, VAO/VBO/EBO; без fixed-function | Pending final verification | |
| R2 | Минимум 5 различных 3D-объектов | Pending final verification | |
| R3 | Корректные MVP и parent–child | Pending final verification | |
| R4 | Per-fragment Phong/Blinn-Phong и минимум 2 источника, один динамический/переключаемый | Pending final verification | |
| R5 | Корректное текстурирование с UV и mipmapping либо разрешённый эквивалент | Pending final verification | |
| R6 | Мышь+клавиатура и минимум 2 взаимодействия | Pending final verification | |
| R7 | Стабильные 30+ FPS на TA при 1280×720, счётчик | Pending final verification | |
| R8 | Сборка как на курсе и точный README | Pending final verification | |
| A1 | 4 | Pending final verification | |
| A2 | 5 | Pending final verification | |
| A3 | 6 | Pending final verification | |
| A4 | 5 | Pending final verification | |
| A5 | 8 | Pending final verification | |
| A6 | 2 | Pending final verification | |
| B1 | 5 | Pending final verification | |
| B2 | 4 | Pending final verification | |
| B3 | 4 | Pending final verification | |
| B4 | 2 | Pending final verification | |
| C1 | 4 | Pending final verification | |
| C2 | 4 | Pending final verification | |
| C3 | 2 | Pending final verification | |
| D1 | 3 | Pending final verification | |
| D2 | 3 | Pending final verification | |
| D3 | 2 | Pending final verification | |
| D4 | 2 | Pending final verification | |
| E1 | 2 | Pending final verification | |
| E2 | 2 | Pending final verification | |
| E3 | 3 | Pending final verification | |
| E4 | 3 | Pending final verification | |
| E5 | 2 | Pending final verification | |
| E6 | 1 | Pending final verification | |
| E7 | 2 | Pending final verification | |
| F1 | 3 | Pending final verification | |
| F2 | 2 | Pending final verification | |
| F3 | 2 | Pending final verification | |
| F4 | 3 | Pending final verification | |
| G1 | 1 | Pending final verification | |
| G2 | 3 | Pending final verification | |
| G3 | 2 | Pending final verification | |
| G4 | 2 | Pending final verification | |
| G5 | 2 | Pending final verification | |

## Verified first checkpoint

- C++17 / OpenGL 3.3 core build succeeds with project-local LLVM-MinGW and CMake.
- The application rendered 360 frames on the RTX 5060 without OpenGL errors.
- Initial math/game tests cover boundary and rotated point/box tests, standing/crouching, alarm, reset, pause and safe victory.
- Screenshot: evidence/screenshots/checkpoint-greybox.png. This is a greybox, not final art.
- Raw diagnostics: evidence/runtime-info.txt.

## Pending external assessment

TA hardware performance, exact course-starter compatibility, live oral defense and the author's understanding cannot be certified by local automated checks.
