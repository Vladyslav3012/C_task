# MENU_launcher — система меню (C + GTK3)

Графічне меню для запуску прикладних програм (завдання з листа І. Чайковського).

**Git-репозиторій тільки в цій папці (`app/`).**  
Каталог `Ccc/` вище — для лабораторних і інших файлів; вони **не** входять у git.

## Структура

```
app/
├── Makefile
├── README.md
├── .gitignore
├── config/
│   └── programs.conf
├── include/
│   └── programs.h
├── src/
│   ├── core/
│   │   └── programs.c
│   └── launcher/
│       ├── gui.c
│       └── cli.c
├── modules/
│   ├── demo/main.c
│   └── gold_stub/main.c
├── bin/          ← після make (не в git)
└── output/       ← звіти PRINT (не в git)
```

## Збірка

```bash
cd app
sudo apt install build-essential libgtk-3-dev   # один раз
make
make run
```

Консоль: `make run-cli`

## Git

```bash
cd app
git status
git add .
git commit -m "Початкова версія меню-лаунчера"
```

## Нова прикладна програма

1. `modules/my_tool/main.c`
2. Ціль у `Makefile` (як `demo`)
3. Рядок у `config/programs.conf`: `Назва|./bin/my_tool|Опис`
4. `make` → **RESET** у вікні

## Зовнішня програма (Python)

Скрипт `bin/run_gold.sh` + запис у `config/programs.conf` — див. **USER MANUAL** у вікні.
