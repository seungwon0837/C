/*
 * 실습과제: 패스워드 유효성 검사
 * -----------------------------------------------------------------------------
 *  - main()에서 패스워드를 입력받아 verification_pw()로 유효성을 검사하고
 *    결과(유효/무효 및 사유)를 출력한다.
 *  - "계속<y/n>" 선택에서 'n'이 입력될 때까지 반복 수행한다.
 *  - 패스워드 입력 시 사용자 입력문자 대신 '*'를 에코로 출력한다.
 *
 *  [유효성 검사 조건]
 *    1) 영숫자 및 특수문자(!, @, #, $) 로만 구성
 *    2) 길이는 최소 8자리, 최대 20자리
 *    3) 소문자, 대문자, 숫자, 특수문자를 각각 1개 이상 반드시 포함
 *    4) 3자리 이상의 오름차순/내림차순 연속 문자열을 포함하면 무효 처리
 *
 *  [제약 사항]
 *    - 전역변수 사용 금지, goto문 사용 금지
 *    - 유효성 검사 함수 원형: int verification_pw(char pw[], int size);
 *
 *  Windows(_WIN32)와 POSIX(Linux/macOS) 양쪽에서 '*' 마스킹 입력이 동작하도록
 *  플랫폼별 처리를 분리하였다.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>          /* _getch() */
#else
#include <termios.h>        /* tcgetattr / tcsetattr */
#include <unistd.h>         /* STDIN_FILENO */
#endif

/* ---- 상수 정의 (전역변수가 아닌 매크로/열거형으로 처리) -------------------- */

#define PW_MIN_LEN   8      /* 허용 최소 길이 */
#define PW_MAX_LEN   20     /* 허용 최대 길이 */

/*
 * 입력 버퍼는 최대 길이(20)보다 넉넉하게 잡는다.
 * 이렇게 해야 "20자리 초과" 같은 무효 케이스도 실제로 입력하여 검사할 수 있고,
 * 버퍼 오버플로우도 방어할 수 있다.
 */
#define PW_BUF_SIZE  64

/* verification_pw()의 반환값(유효성 검사 결과 코드) */
enum pw_result {
    PW_VALID = 0,           /* 유효 */
    PW_ERR_NULL,            /* 잘못된 인자(방어용) */
    PW_ERR_TOO_SHORT,       /* 길이 미달 */
    PW_ERR_TOO_LONG,        /* 길이 초과 */
    PW_ERR_BAD_CHAR,        /* 허용되지 않는 문자 포함 */
    PW_ERR_NO_LOWER,        /* 소문자 없음 */
    PW_ERR_NO_UPPER,        /* 대문자 없음 */
    PW_ERR_NO_DIGIT,        /* 숫자 없음 */
    PW_ERR_NO_SPECIAL,      /* 특수문자 없음 */
    PW_ERR_SEQUENCE         /* 3자리 이상 오름/내림차순 연속 */
};

/* ---- 함수 원형 ------------------------------------------------------------ */

int verification_pw(char pw[], int size);                 /* 과제 지정 원형 */

static int  is_special_char(int ch);
static int  has_ascending_or_descending_run(const char pw[], int size);
static const char *result_reason(int code);
static int  read_password(char buf[], int cap);
static int  ask_continue(void);

/* ---- 유효성 검사 (핵심 로직) ---------------------------------------------- */

/*
 * 허용되는 특수문자(!, @, #, $) 여부를 판정한다.
 */
static int is_special_char(int ch)
{
    return (ch == '!' || ch == '@' || ch == '#' || ch == '$');
}

/*
 * 3자리 이상의 오름차순 또는 내림차순 연속 문자열이 있는지 검사한다.
 * 연속의 기준은 ASCII 코드값의 차이가 1인 경우이다. (예: "abc", "789", "321")
 * 하나라도 발견되면 1, 없으면 0을 반환한다.
 */
static int has_ascending_or_descending_run(const char pw[], int size)
{
    int i;

    for (i = 0; i + 2 < size; i++) {
        int a = (unsigned char)pw[i];
        int b = (unsigned char)pw[i + 1];
        int c = (unsigned char)pw[i + 2];

        if (b - a == 1 && c - b == 1)   /* 오름차순 (예: a,b,c) */
            return 1;
        if (a - b == 1 && b - c == 1)   /* 내림차순 (예: c,b,a) */
            return 1;
    }
    return 0;
}

/*
 * 패스워드 유효성 검사 함수.
 *   pw   : 검사할 패스워드 문자열
 *   size : 패스워드의 자리수(길이)
 *   반환 : enum pw_result 값 (PW_VALID이면 유효, 그 외에는 무효 사유 코드)
 */
int verification_pw(char pw[], int size)
{
    int i;
    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0;

    if (pw == NULL)                 /* 방어적 처리 */
        return PW_ERR_NULL;

    /* 조건 2) 길이 검사 */
    if (size < PW_MIN_LEN)
        return PW_ERR_TOO_SHORT;
    if (size > PW_MAX_LEN)
        return PW_ERR_TOO_LONG;

    /* 조건 1) 허용 문자 검사 + 조건 3) 문자 종류별 포함 여부 집계 */
    for (i = 0; i < size; i++) {
        unsigned char ch = (unsigned char)pw[i];

        if (islower(ch))
            has_lower = 1;
        else if (isupper(ch))
            has_upper = 1;
        else if (isdigit(ch))
            has_digit = 1;
        else if (is_special_char(ch))
            has_special = 1;
        else
            return PW_ERR_BAD_CHAR;     /* 허용되지 않는 문자 */
    }

    /* 조건 3) 각 문자 종류가 1개 이상 포함되었는지 확인 */
    if (!has_lower)
        return PW_ERR_NO_LOWER;
    if (!has_upper)
        return PW_ERR_NO_UPPER;
    if (!has_digit)
        return PW_ERR_NO_DIGIT;
    if (!has_special)
        return PW_ERR_NO_SPECIAL;

    /* 조건 4) 오름/내림차순 연속 문자열 검사 */
    if (has_ascending_or_descending_run(pw, size))
        return PW_ERR_SEQUENCE;

    return PW_VALID;
}

/*
 * 무효 사유 코드에 대응하는 설명 문자열을 반환한다.
 */
static const char *result_reason(int code)
{
    switch (code) {
    case PW_ERR_NULL:       return "입력값이 없습니다";
    case PW_ERR_TOO_SHORT:  return "길이가 8자리 미만입니다";
    case PW_ERR_TOO_LONG:   return "길이가 20자리를 초과합니다";
    case PW_ERR_BAD_CHAR:   return "영숫자와 특수문자(!,@,#,$)만 사용할 수 있습니다";
    case PW_ERR_NO_LOWER:   return "소문자가 포함되어 있지 않습니다";
    case PW_ERR_NO_UPPER:   return "대문자가 포함되어 있지 않습니다";
    case PW_ERR_NO_DIGIT:   return "숫자가 포함되어 있지 않습니다";
    case PW_ERR_NO_SPECIAL: return "특수문자(!,@,#,$)가 포함되어 있지 않습니다";
    case PW_ERR_SEQUENCE:   return "3자리 이상 연속된 오름/내림차순 문자열이 있습니다";
    default:                return "알 수 없는 오류";
    }
}

/* ---- 입력 처리 (I/O) ------------------------------------------------------ */

/*
 * 패스워드를 한 글자씩 입력받아 화면에는 '*'로 마스킹하여 출력한다.
 * 백스페이스(지우기)도 지원한다.
 *   buf : 입력을 저장할 버퍼
 *   cap : 버퍼 용량(널 문자 포함)
 *   반환: 입력된 문자열의 길이. 입력 없이 EOF를 만나면 -1.
 *
 * 터미널 제어가 불가능한 환경(예: 파이프 입력)에서는 마스킹 설정 없이도
 * 동작하도록 방어적으로 처리한다.
 */
static int read_password(char buf[], int cap)
{
    int len = 0;
    int ch;

#ifndef _WIN32
    struct termios old_term, new_term;
    int raw_mode = 0;

    /* 표준입력의 에코/정규 모드를 잠시 해제하여 직접 마스킹한다. */
    if (tcgetattr(STDIN_FILENO, &old_term) == 0) {
        new_term = old_term;
        new_term.c_lflag &= ~(tcflag_t)(ICANON | ECHO);
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == 0)
            raw_mode = 1;
    }
#endif

    for (;;) {
#ifdef _WIN32
        ch = _getch();
        if (ch == '\r' || ch == '\n')           /* Enter */
            break;
#else
        ch = getchar();
        if (ch == '\n' || ch == '\r' || ch == EOF)
            break;
#endif
        if (ch == '\b' || ch == 127) {          /* Backspace / Delete */
            if (len > 0) {
                len--;
                fputs("\b \b", stdout);         /* 화면의 '*' 한 칸 지우기 */
                fflush(stdout);
            }
            continue;
        }

        if (len < cap - 1) {                     /* 버퍼 한도 내에서만 저장 */
            buf[len++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
        /* 한도를 넘는 입력은 무시(소비)하여 오버플로우를 방지한다. */
    }

    buf[len] = '\0';

#ifndef _WIN32
    if (raw_mode)
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);  /* 터미널 모드 복원 */
    if (ch == EOF && len == 0)
        return -1;
#endif

    putchar('\n');
    return len;
}

/*
 * "계속<y/n>"을 묻고 사용자의 선택을 받는다.
 *   반환: 계속하면 1, 종료하면 0.
 * 'y'/'n'(대소문자 무관) 외의 값이 들어오면 다시 묻는다(방어적 처리).
 */
static int ask_continue(void)
{
    for (;;) {
        int ch, rest;

        printf("   계속<y/n> : ");
        fflush(stdout);

        /* 앞쪽 공백/개행은 건너뛰고 첫 의미 있는 문자를 읽는다. */
        do {
            ch = getchar();
        } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

        if (ch == EOF)
            return 0;

        /* 입력 줄의 나머지 문자를 비운다. */
        rest = ch;
        while (rest != '\n' && rest != EOF)
            rest = getchar();

        if (ch == 'n' || ch == 'N')
            return 0;
        if (ch == 'y' || ch == 'Y')
            return 1;

        printf("   'y' 또는 'n'으로 입력해 주세요.\n");
    }
}

/* ---- main ----------------------------------------------------------------- */

int main(void)
{
    char pw[PW_BUF_SIZE];
    int count = 0;          /* 총 검사 횟수 */
    int valid_count = 0;    /* 유효 판정 횟수 */
    int len, result;

    printf("[ 패스워드 유효성 검사 ]\n\n");

    for (;;) {
        count++;
        printf("%d. 검사할 패스워드를 입력하시오 : ", count);
        fflush(stdout);

        len = read_password(pw, PW_BUF_SIZE);
        if (len < 0) {                  /* 입력 없이 EOF: 검사 취소 후 종료 */
            count--;
            putchar('\n');
            break;
        }

        result = verification_pw(pw, len);
        if (result == PW_VALID) {
            valid_count++;
            printf("   입력한 \"%s\"은 유효한 암호입니다.\n", pw);
        } else {
            printf("   입력한 \"%s\"은 유효한 암호가 아닙니다. (사유: %s)\n",
                   pw, result_reason(result));
        }

        if (!ask_continue())
            break;
    }

    printf("\n총 %d회 검사 중 %d회 유효 !!\n", count, valid_count);
    printf("[ 패스워드 유효성 검사 종료 ]\n");

    return 0;
}
