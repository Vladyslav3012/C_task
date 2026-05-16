/*
 * Заглушка для місця програми GOLD_code_correlation_analysis.
 * Коли буде готовий виконуваний файл (C або Python-обгортка),
 * вкажіть його шлях у programs.conf замість ./bin/gold_stub
 */

#include <stdio.h>

int main(void)
{
    printf("\n");
    printf("  Програма [GOLD_code_correlation_analysis]\n");
    printf("  ----------------------------------------\n");
    printf("  Це заглушка. Підключіть готову програму:\n");
    printf("  1. Зберіть або скопіюйте виконуваний файл.\n");
    printf("  2. Відредагуйте config/programs.conf — рядок з Gold codes.\n");
    printf("     Приклад для Python:\n");
    printf("     Gold analysis|python3 /шлях/gold_main.py|Аналіз кодів Голда\n");
    printf("  ----------------------------------------\n\n");
    return 0;
}
