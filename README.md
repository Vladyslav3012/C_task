# MENU_launcher — система меню (C + GTK3)

Графічне меню для запуску прикладних програм.

**Git-репозиторій тільки в `app/`.** Папка `Ccc/` з лабами — окремо, не в git.

## Структура

```
app/
├── build.bat, run.bat      ← Windows (подвійний клік)
├── Makefile
├── config/programs.conf
├── include/
├── src/core/               programs.c, process.c
├── src/launcher/           gui.c, cli.c
├── modules/demo/, gold_stub/
├── bin/                    після make
└── output/
```

## Linux

```bash
cd app
sudo apt install build-essential libgtk-3-dev
make
make run
```

## Windows (без WSL)

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

Шляхи в `config/programs.conf` можна лишати `./bin/demo` — на Windows автоматично шукається `demo.exe`.
``

## Нова програма

1. `modules/my_tool/main.c` + ціль у `Makefile`
2. `config/programs.conf`: `Назва|./bin/my_tool|Опис`
3. `make` → **RESET**

## Python / зовнішні exe

Додайте `.exe` або `.bat` у `bin/` і пропишіть у `config/programs.conf`.
