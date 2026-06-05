#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int customers[20][2];
// customers[i][0] : 비밀번호
// customers[i][1] : 잔고

void init_data(void);
int system_input(const char* message, int min, int max);
int money_input(const char* message);

int start_screen(void);
void main_menu(int index);

void check_balance(int index);
void deposit_money(int index);
void withdraw_money(int index);
void change_password(int index);

int main(void)
{
    int index;

    init_data();

    while (1)
    {
        index = start_screen();

        if (index == -1)
        {
            printf("프로그램을 종료합니다.\n");
            return 0;
        }

        main_menu(index);
    }
}

void init_data(void)
{
    for (int i = 0; i < 20; i++)
    {
        customers[i][0] = i + 1;
        customers[i][1] = 10000;
    }
}

int system_input(const char* message, int min, int max)
{
    int value;

    while (1)
    {
        printf("%s", message);

        if (scanf("%d", &value) != 1)
        {
            while (getchar() != '\n');
            printf("숫자를 입력하세요.\n");
            continue;
        }

        if (value < min || value > max)
        {
            printf("%d ~ %d 사이의 값을 입력하세요.\n", min, max);
            continue;
        }

        return value;
    }
}

int money_input(const char* message)
{
    int value;

    while (1)
    {
        printf("%s", message);

        if (scanf("%lld", &value) != 1)
        {
            while (getchar() != '\n');
            printf("숫자를 입력하세요.\n");
            continue;
        }

        if (value <= 0)
        {
            printf("금액은 0보다 커야 합니다.\n");
            continue;
        }

        if (value > INT_MAX)
        {
            printf("입력한 금액이 너무 큽니다.\n");
            continue;
        }

        return (int)value;
    }
}

int start_screen(void)
{
    int account_num;
    int password;
    int index;

    while (1)
    {
        printf("\n*** Welcome to 콘서트 ATM ***\n\n");
        printf("고객번호를 입력하세요! (1~20)\n");

        account_num = system_input("(종료하려면 0을 입력) : ", 0, 20);

        if (account_num == 0)
        {
            return -1;
        }

        index = account_num - 1;

        password = system_input("비밀번호 입력: ", 1, 99);

        if (customers[index][0] == password)
        {
            printf("\n고객 인증 완료.\n");
            printf("%d번 고객님 환영합니다.\n", account_num);
            return index;
        }

        printf("비밀번호가 일치하지 않습니다.\n");
    }
}

void main_menu(int index)
{
    int menu;

    while (1)
    {
        printf("\n**** 하나를 선택하시오 ****\n");
        printf("계좌번호: %d\n", index + 1);
        printf("<0> 프로그램 종료\n");
        printf("<1> 잔고 확인\n");
        printf("<2> 입금\n");
        printf("<3> 인출\n");
        printf("<5> 비밀번호 변경\n");
        printf("<9> 시작 화면으로 돌아가기\n");

        menu = system_input("메뉴 선택: ", 0, 9);

        switch (menu)
        {
        case 0:
            printf("프로그램을 종료합니다.\n");
            exit(0);

        case 1:
            check_balance(index);
            break;

        case 2:
            deposit_money(index);
            break;

        case 3:
            withdraw_money(index);
            break;

        case 5:
            change_password(index);
            break;

        case 9:
            printf("시작 화면으로 돌아갑니다.\n");
            return;

        default:
            printf("올바른 메뉴를 선택하세요.\n");
        }
    }
}

void check_balance(int index)
{
    printf("\n--- [잔고확인] ---\n");
    printf("계좌번호: %d\n", index + 1);
    printf("현재 잔고: %d원\n", customers[index][1]);
}

void deposit_money(int index)
{
    int money;

    printf("\n--- [입금] ---\n");

    money = money_input("입금할 금액 입력: ");

    if (customers[index][1] > INT_MAX - money)
    {
        printf("입금 후 잔고가 저장 가능한 범위를 초과합니다.\n");
        printf("현재 잔고: %d원\n", customers[index][1]);
        return;
    }

    customers[index][1] += money;

    printf("%d원이 입금되었습니다.\n", money);
    printf("현재 잔고: %d원\n", customers[index][1]);
}

void withdraw_money(int index)
{
    int money;

    printf("\n--- [출금] ---\n");

    money = money_input("출금할 금액 입력: ");

    if (money > customers[index][1])
    {
        printf("잔고가 부족합니다.\n");
        printf("현재 잔고: %d원\n", customers[index][1]);
        return;
    }

    customers[index][1] -= money;

    printf("%d원이 출금되었습니다.\n", money);
    printf("현재 잔고: %d원\n", customers[index][1]);
}

void change_password(int index)
{
    int current_password;
    int new_password;

    printf("\n--- [비밀번호 변경] ---\n");

    current_password = system_input("현재 비밀번호 입력: ", 1, 99);

    if (current_password != customers[index][0])
    {
        printf("현재 비밀번호가 일치하지 않습니다.\n");
        return;
    }

    new_password = system_input("새 비밀번호 입력: ", 1, 99);

    customers[index][0] = new_password;

    printf("비밀번호가 변경되었습니다.\n");
}
