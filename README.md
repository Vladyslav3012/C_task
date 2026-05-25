# GOLD_code_correlation_analysis — меню (C + GTK3)

Графічне меню для аналізу кодів Голда (інтерфейс 1:1 за макетом) з підключенням обчислювальних модулів.

## Структура

```
app/
├── config/gold.conf        шляхи compute / viewer
├── docs/user_manual_uk.txt
├── src/launcher/           gui.c, gui_layout.c, gui_handlers.c, …
├── src/core/               programs.c, process.c, gold_config.c
├── modules/gold_compute/   заглушка обчислення
├── modules/gold_viewer/    перегляд результатів (GTK)
├── bin/                    після make
└── output/                 summary.json, acf_*.txt, report.txt
```

## Linux

```bash
cd app
sudo apt install build-essential libgtk-3-dev
make
make run
```

## Windows

Потрібна **окрема програма — [MSYS2](https://www.msys2.org/)** (~500 МБ).  
Це середовище з компілятором C, `make` і GTK (як `apt` у Linux). Без нього `.exe` не зібрати.

### Крок 1 — один раз

1. Завантажте і встановіть **MSYS2** → за замовчуванням `C:\msys64`
2. Подвійний клік **`build.bat`** у папці `app`  
   (або термінал **MSYS2 UCRT64**):

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-gtk3 make
cd /c/шлях/до/Ccc/app
make
```

### Крок 2 — запуск

- Подвійний клік **`run.bat`**, або  
- У UCRT64: `make run`

`run.bat` додає `C:\msys64\ucrt64\bin` у PATH (DLL для GTK). Якщо MSYS2 в іншому місці — змініть шлях у `run.bat`.

### Що не підійде

| Не достатньо | Чому |
|--------------|------|
| Лише «Командний рядок» Windows | немає `gcc`, `make`, GTK |
| Visual Studio без GTK | немає gtk+-3.0 |
| Подвійний клік `launcher.exe` без `run.bat` | не знайде DLL GTK |

## Кнопки

| Кнопка | Дія |
|--------|-----|
| USER MANUAL | Посібник з docs/user_manual_uk.txt |
| INFO | Поточні параметри та шляхи |
| START | Запуск gold_compute з параметрами форми |
| FINISH | Вихід (з підтвердженням при обчисленні) |
| RESET | Скидання форми та output |
| PRINT | output/report.txt |
| Опції | config/gold.conf |
| АКФ/ВКФ/SRT | gold_viewer для відповідного файлу |

## Підключення зовнішньої програми

У `config/gold.conf`:

```
compute=./bin/your_engine.exe
viewer=./bin/gold_viewer
```

Обчислювальна програма повинна після роботи створити:

- `output/summary.json` — поля prim_base, prim_opt, pairs_base, pairs_opt, ens1, ens2, ens3
- `output/acf_1.txt` … `output/usrt_2.txt`

Аргументи командного рядка: `--n`, `--corr`, `--crit`, `--k` (як у gold_compute).

Для Python або іншого `.exe` вкажіть повний шлях у `compute=`; робочий каталог — каталог `app/` (`LAUNCHER_HOME`).
