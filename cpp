#define _CRT_SECURE_NO_WARNINGS
#ifndef RGBA
#define RGBA(r,g,b,a) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#endif

#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>

using namespace std;

// ===================== 全局常量定义 =====================
// 场景枚举
enum Scene {
    SCENE_CLASSROOM,   // 教室主场景
    SCENE_EXAM,        // 月考/高考考场场景
    SCENE_VOLUNTEER,   // 志愿填报场景
    SCENE_ENDING       // 结局场景
};

// 科目枚举
enum Subject {
    CHINESE,    // 语文
    MATH,       // 数学
    ENGLISH,    // 英语
    SCIENCE,    // 理综
    SUBJECT_COUNT
};

// 题目结构体
struct Question {
    string title;
    string options[4];
    int correctIndex;
    Subject subject;
};

// 按钮/互动区域结构体
struct ClickArea {
    int x, y, width, height;
    string text;
    bool isHover;    // 鼠标是否悬停
    bool isEnable;   // 是否可点击
};

// ===================== 全局游戏状态结构体 =====================
struct GameState {
    // 基础游戏状态
    int day;                    // 当前天数（共30天）
    int subjectScore[SUBJECT_COUNT]; // 各科分数
    int totalScore;             // 高考总分
    int fatigue;                // 疲劳值 0-100
    Scene currentScene;         // 当前场景
    bool gameOver;              // 游戏是否结束
    int volunteerChoice;        // 志愿选择
    string eventText;           // 事件提示文本
    string resultText;          // 结局文本
    int mouseX, mouseY;         // 实时鼠标位置

    // 答题相关状态（之前漏了，导致报错）
    bool isAnswering;           // 是否正在答题
    Question currentQuestion;   // 当前题目
    int answerTimeLeft;         // 答题剩余时间
    bool isMonthlyExam;         // 是否是月考
    bool isFinalExam;           // 是否是高考

    // 摸鱼小游戏相关状态
    bool isPlayingHideGame;     // 是否在玩躲班主任游戏
    int hideGameTimer;          // 游戏倒计时
    bool teacherIsComing;       // 班主任是否来了
};

// ===================== 声明全局game变量 =====================
GameState game;

// ===================== 全局互动区域定义 =====================
// 教室场景互动区
ClickArea areaBook = { 200, 400, 140, 80, "课本\n(刷题答题)", false, true };
ClickArea areaBlackboard = { 300, 100, 200, 120, "黑板\n(认真听课)", false, true };
ClickArea areaPhone = { 500, 450, 100, 50, "手机\n(摸鱼休息)", false, true };
ClickArea areaCalendar = { 650, 100, 100, 80, "日历", false, false };

// 答题选项按钮
ClickArea areaOption[4] = {
    {150, 400, 250, 60, "A. ", false, true},
    {450, 400, 250, 60, "B. ", false, true},
    {150, 480, 250, 60, "C. ", false, true},
    {450, 480, 250, 60, "D. ", false, true}
};

// 志愿填报按钮
ClickArea areaVolunteer[7] = {
    {200, 250, 400, 40, "清华大学/北京大学（需680+）", false, true},
    {200, 300, 400, 40, "985工程大学（需600+）", false, true},
    {200, 350, 400, 40, "211工程大学（需550+）", false, true},
    {200, 400, 400, 40, "普通一本大学（需500+）", false, true},
    {200, 450, 400, 40, "二本/三本大学（需400+）", false, true},
    {200, 500, 400, 40, "专科学校（需200+）", false, true},
    {200, 550, 400, 40, "复读一年", false, true}
};

// ===================== 题库=====================
Question questionBank[12] = {
    {"鲁迅的原名是？", {"周树人", "周作人", "周建人", "周恩来"}, 0, CHINESE},
    {"1+2*3+4的计算结果是？", {"13", "11", "15", "9"}, 1, MATH},
    {"以下哪个是英语元音字母？", {"B", "C", "A", "D"}, 2, ENGLISH},
    {"水的化学式是？", {"H2O", "CO2", "NaCl", "O2"}, 0, SCIENCE},
    {"《静夜思》的作者是？", {"杜甫", "李白", "白居易", "王维"}, 1, CHINESE},
    {"圆的面积公式是？", {"π乘以半径", "2乘以π乘以半径", "π乘以半径的平方", "半径的平方"}, 2, MATH},
    {"\"Thank you\"的正确回答是？", {"No thanks", "You're welcome", "Sorry", "Goodbye"}, 1, ENGLISH},
    {"牛顿第一定律又被称为？", {"惯性定律", "能量守恒", "万有引力", "热力学定律"}, 0, SCIENCE},
    {"以下哪个是唐宋八大家？", {"李白", "杜甫", "韩愈", "李清照"}, 2, CHINESE},
    {"直角三角形勾股定理是？", {"a+b=c", "a*a + b*b = c*c", "a*b=c", "a/b=c"}, 1, MATH},
    {"英语中一般过去时do的正确变形是？", {"do", "does", "did", "done"}, 2, ENGLISH},
    {"空气中含量最多的气体是？", {"氧气", "氮气", "二氧化碳", "氢气"}, 1, SCIENCE}
};

// ===================== 函数声明 =====================
void InitGame();
void ResetAllHoverState();
void DrawScene();
void HandleMouseClick(int x, int y);
void UpdateHoverState(int x, int y);
void RandomEvent();
void GenerateRandomQuestion();
void StartMonthlyExam();
void StartFinalExam();
void GenerateResult();
void RestartGame();
void UpdateTotalScore();

// ===================== 主函数 =====================
int main() {
    initgraph(800, 650);
    BeginBatchDraw(); // 双缓冲防闪烁
    setbkcolor(WHITE);
    cleardevice();

    InitGame();

    // 主游戏循环
    while (true) {
        // 实时更新鼠标悬停状态
        POINT mouse_msg;
        GetCursorPos(&mouse_msg);
        ScreenToClient(GetHWnd(), &mouse_msg);
        game.mouseX = mouse_msg.x;
        game.mouseY = mouse_msg.y;
        UpdateHoverState(game.mouseX, game.mouseY);

        // 绘制画面
        DrawScene();
        FlushBatchDraw();

        // 答题倒计时
        if (game.currentScene == SCENE_EXAM && game.answerTimeLeft > 0) {
            static int timeCounter = 0;
            timeCounter++;
            if (timeCounter >= 100) { // 10ms*100=1秒
                game.answerTimeLeft--;
                timeCounter = 0;
                if (game.answerTimeLeft <= 0) {
                    game.eventText = "答题超时！本次答题无效";
                    game.currentScene = SCENE_CLASSROOM;
                    game.day++;
                    ResetAllHoverState();
                }
            }
        }

        // 躲班主任游戏倒计时
        if (game.isPlayingHideGame) {
            static int hideCounter = 0;
            hideCounter++;
            if (hideCounter >= 50) { // 500ms
                game.hideGameTimer--;
                hideCounter = 0;
                if (game.hideGameTimer <= 0) {
                    if (game.teacherIsComing) {
                        game.eventText = "被班主任抓到了！扣20分，疲劳+30";
                        game.totalScore -= 20;
                        game.fatigue += 30;
                    }
                    else {
                        game.eventText = "摸鱼成功！疲劳-30";
                        game.fatigue -= 30;
                    }
                    game.isPlayingHideGame = false;
                    game.day++;
                }
            }
        }

        // 鼠标点击处理
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                HandleMouseClick(msg.x, msg.y);
            }
        }

        // ESC退出
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        Sleep(10);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}

// ===================== 游戏初始化 =====================
void InitGame() {
    srand((unsigned int)time(NULL));

    game.day = 1;
    for (int i = 0; i < SUBJECT_COUNT; i++) {
        game.subjectScore[i] = 90;
    }
    UpdateTotalScore();
    game.fatigue = 0;
    game.currentScene = SCENE_CLASSROOM;
    game.gameOver = false;
    game.volunteerChoice = -1;
    game.eventText = "高三开学啦！距离高考还有30天，点击场景内的物品开始行动";
    game.resultText = "";
    game.mouseX = 0;
    game.mouseY = 0;

    // 重置答题状态
    game.isAnswering = false;
    game.answerTimeLeft = 0;
    game.isMonthlyExam = false;
    game.isFinalExam = false;

    // 重置小游戏状态
    game.isPlayingHideGame = false;
    game.hideGameTimer = 0;
    game.teacherIsComing = false;

    ResetAllHoverState();
}

// ===================== 重置所有按钮悬停状态 =====================
void ResetAllHoverState() {
    areaBook.isHover = false;
    areaBlackboard.isHover = false;
    areaPhone.isHover = false;
    areaCalendar.isHover = false;

    for (int i = 0; i < 4; i++) {
        areaOption[i].isHover = false;
        areaOption[i].isEnable = true;
    }

    for (int i = 0; i < 7; i++) {
        areaVolunteer[i].isHover = false;
        areaVolunteer[i].isEnable = true;
    }
}

// ===================== 实时更新悬停状态 =====================
void UpdateHoverState(int x, int y) {
    ResetAllHoverState();

    if (game.currentScene == SCENE_CLASSROOM) {
        areaBook.isHover = (x >= areaBook.x && x <= areaBook.x + areaBook.width && y >= areaBook.y && y <= areaBook.y + areaBook.height && areaBook.isEnable);
        areaBlackboard.isHover = (x >= areaBlackboard.x && x <= areaBlackboard.x + areaBlackboard.width && y >= areaBlackboard.y && y <= areaBlackboard.y + areaBlackboard.height && areaBlackboard.isEnable);
        areaPhone.isHover = (x >= areaPhone.x && x <= areaPhone.x + areaPhone.width && y >= areaPhone.y && y <= areaPhone.y + areaPhone.height && areaPhone.isEnable);
    }
    else if (game.currentScene == SCENE_EXAM) {
        for (int i = 0; i < 4; i++) {
            areaOption[i].isHover = (x >= areaOption[i].x && x <= areaOption[i].x + areaOption[i].width && y >= areaOption[i].y && y <= areaOption[i].y + areaOption[i].height && areaOption[i].isEnable);
        }
    }
    else if (game.currentScene == SCENE_VOLUNTEER) {
        for (int i = 0; i < 7; i++) {
            areaVolunteer[i].isHover = (x >= areaVolunteer[i].x && x <= areaVolunteer[i].x + areaVolunteer[i].width && y >= areaVolunteer[i].y && y <= areaVolunteer[i].y + areaVolunteer[i].height && areaVolunteer[i].isEnable);
        }
    }
}

// ===================== 场景绘制 =====================
void DrawScene() {
    cleardevice();
    settextcolor(BLACK);

    // 1. 教室主场景
    if (game.currentScene == SCENE_CLASSROOM) {
        setfillcolor(RGB(240, 240, 240));
        fillrectangle(0, 0, 800, 650);

        // 绘制黑板
        COLORREF boardColor = areaBlackboard.isHover ? RGB(60, 60, 60) : RGB(40, 40, 40);
        setfillcolor(boardColor);
        fillrectangle(areaBlackboard.x, areaBlackboard.y, areaBlackboard.x + areaBlackboard.width, areaBlackboard.y + areaBlackboard.height);
        settextcolor(WHITE);
        settextstyle(24, 0, "黑体");
        outtextxy(330, 140, "高考倒计时");
        char countdown[20];
        sprintf(countdown, "%d天", 31 - game.day);
        outtextxy(360, 180, countdown);

        // 绘制课桌
        settextcolor(BLACK);
        setfillcolor(RGB(139, 90, 43));
        fillrectangle(100, 380, 700, 500);
        fillrectangle(50, 500, 750, 650);

        // 绘制课本
        COLORREF bookColor = areaBook.isHover ? RGB(255, 240, 180) : RGB(255, 250, 240);
        setfillcolor(bookColor);
        fillrectangle(areaBook.x, areaBook.y, areaBook.x + areaBook.width, areaBook.y + areaBook.height);
        setlinecolor(areaBook.isHover ? RED : BLACK);
        setlinestyle(PS_SOLID, areaBook.isHover ? 3 : 1);
        rectangle(areaBook.x, areaBook.y, areaBook.x + areaBook.width, areaBook.y + areaBook.height);
        setlinestyle(PS_SOLID, 1);
        settextstyle(16, 0, "宋体");
        settextcolor(BLACK);
        outtextxy(areaBook.x + 20, areaBook.y + 20, areaBook.text.c_str());

        // 绘制手机
        COLORREF phoneColor = areaPhone.isHover ? RGB(60, 60, 60) : RGB(30, 30, 30);
        setfillcolor(phoneColor);
        fillrectangle(areaPhone.x, areaPhone.y, areaPhone.x + areaPhone.width, areaPhone.y + areaPhone.height);
        setlinecolor(areaPhone.isHover ? RED : BLACK);
        setlinestyle(PS_SOLID, areaPhone.isHover ? 3 : 1);
        rectangle(areaPhone.x, areaPhone.y, areaPhone.x + areaPhone.width, areaPhone.y + areaPhone.height);
        setlinestyle(PS_SOLID, 1);
        settextcolor(WHITE);
        outtextxy(areaPhone.x + 5, areaPhone.y + 10, areaPhone.text.c_str());

        // 绘制日历
        settextcolor(BLACK);
        setfillcolor(WHITE);
        fillrectangle(areaCalendar.x, areaCalendar.y, areaCalendar.x + areaCalendar.width, areaCalendar.y + areaCalendar.height);
        rectangle(areaCalendar.x, areaCalendar.y, areaCalendar.x + areaCalendar.width, areaCalendar.y + areaCalendar.height);
        settextstyle(16, 0, "宋体");
        char dayText[20];
        sprintf(dayText, "第%d天", game.day);
        outtextxy(areaCalendar.x + 20, areaCalendar.y + 20, dayText);
        outtextxy(areaCalendar.x + 10, areaCalendar.y + 45, areaCalendar.text.c_str());

        // 绘制状态栏
        settextstyle(20, 0, "宋体");
        settextcolor(BLACK);
        char status[150];
        sprintf(status, "总分: %d | 语文:%d 数学:%d 英语:%d 理综:%d | 疲劳值: %d/100",
            game.totalScore, game.subjectScore[CHINESE], game.subjectScore[MATH],
            game.subjectScore[ENGLISH], game.subjectScore[SCIENCE], game.fatigue);
        outtextxy(50, 50, status);

        // 绘制事件提示
        settextstyle(22, 0, "宋体");
        outtextxy(100, 300, game.eventText.c_str());

        // 操作提示
        settextstyle(16, 0, "宋体");
        settextcolor(DARKGRAY);
        outtextxy(250, 520, "操作提示：点击课本刷题 | 点击黑板听课 | 点击手机摸鱼");
    }

    // 2. 考场场景
    else if (game.currentScene == SCENE_EXAM) {
        setbkcolor(WHITE);
        cleardevice();

        // 标题
        settextstyle(36, 0, "黑体");
        settextcolor(BLACK);
        outtextxy(300, 30, game.day == 30 ? "高 考 考 场" : "月 考 考 场");

        // 题目
        settextstyle(24, 0, "宋体");
        outtextxy(100, 150, game.currentQuestion.title.c_str());

        // 倒计时
        char timeText[50];
        sprintf(timeText, "剩余时间：%d秒", game.answerTimeLeft);
        settextcolor(game.answerTimeLeft <= 5 ? RED : BLACK);
        outtextxy(600, 150, timeText);

        // 绘制选项
        settextstyle(20, 0, "宋体");
        for (int i = 0; i < 4; i++) {
            COLORREF btnColor = areaOption[i].isHover ? RGB(255, 255, 100) : WHITE;
            setfillcolor(btnColor);
            fillrectangle(areaOption[i].x, areaOption[i].y, areaOption[i].x + areaOption[i].width, areaOption[i].y + areaOption[i].height);

            setlinecolor(areaOption[i].isHover ? RED : BLACK);
            setlinestyle(PS_SOLID, areaOption[i].isHover ? 3 : 1);
            rectangle(areaOption[i].x, areaOption[i].y, areaOption[i].x + areaOption[i].width, areaOption[i].y + areaOption[i].height);
            setlinestyle(PS_SOLID, 1);

            settextcolor(areaOption[i].isHover ? RED : BLACK);
            string optionText = areaOption[i].text + game.currentQuestion.options[i];
            outtextxy(areaOption[i].x + 20, areaOption[i].y + 15, optionText.c_str());
        }

        // 操作提示
        settextstyle(18, 0, "宋体");
        settextcolor(DARKGRAY);
        outtextxy(250, 580, "鼠标移到选项上会高亮变红，点击即可作答");
    }

    // 3. 志愿填报场景
    else if (game.currentScene == SCENE_VOLUNTEER) {
        setbkcolor(WHITE);
        cleardevice();

        settextstyle(36, 0, "黑体");
        settextcolor(BLACK);
        outtextxy(250, 30, "高考志愿填报系统");

        char finalScore[50];
        sprintf(finalScore, "你的高考最终成绩：%d 分", game.totalScore);
        settextstyle(28, 0, "宋体");
        outtextxy(250, 100, finalScore);

        settextstyle(20, 0, "宋体");
        outtextxy(200, 160, "请选择你要填报的第一志愿（分数不够会滑档调剂）：");

        for (int i = 0; i < 7; i++) {
            bool isAvailable = true;
            if (i == 0 && game.totalScore < 680) isAvailable = false;
            else if (i == 1 && game.totalScore < 600) isAvailable = false;
            else if (i == 2 && game.totalScore < 550) isAvailable = false;
            else if (i == 3 && game.totalScore < 500) isAvailable = false;
            else if (i == 4 && game.totalScore < 400) isAvailable = false;
            else if (i == 5 && game.totalScore < 200) isAvailable = false;
            areaVolunteer[i].isEnable = isAvailable;

            COLORREF btnColor;
            if (!isAvailable) btnColor = LIGHTGRAY;
            else if (areaVolunteer[i].isHover) btnColor = RGB(200, 230, 255);
            else btnColor = WHITE;

            setfillcolor(btnColor);
            fillrectangle(areaVolunteer[i].x, areaVolunteer[i].y, areaVolunteer[i].x + areaVolunteer[i].width, areaVolunteer[i].y + areaVolunteer[i].height);

            setlinecolor((areaVolunteer[i].isHover && isAvailable) ? BLUE : BLACK);
            setlinestyle(PS_SOLID, (areaVolunteer[i].isHover && isAvailable) ? 3 : 1);
            rectangle(areaVolunteer[i].x, areaVolunteer[i].y, areaVolunteer[i].x + areaVolunteer[i].width, areaVolunteer[i].y + areaVolunteer[i].height);
            setlinestyle(PS_SOLID, 1);

            settextcolor(isAvailable ? BLACK : DARKGRAY);
            int textX = areaVolunteer[i].x + (areaVolunteer[i].width - textwidth(areaVolunteer[i].text.c_str())) / 2;
            int textY = areaVolunteer[i].y + (areaVolunteer[i].height - textheight(areaVolunteer[i].text.c_str())) / 2;
            outtextxy(textX, textY, areaVolunteer[i].text.c_str());
        }
    }

    // 4. 结局场景
    else if (game.currentScene == SCENE_ENDING) {
        setbkcolor(WHITE);
        cleardevice();

        setfillcolor(RGBA(255, 255, 255, 220));
        fillrectangle(100, 150, 700, 600);
        setlinecolor(BLACK);
        rectangle(100, 150, 700, 600);

        settextstyle(30, 0, "黑体");
        settextcolor(BLACK);
        outtextxy(300, 180, "录取结果公布！");

        settextstyle(20, 0, "宋体");
        int y = 230;
        string line;
        for (char c : game.resultText) {
            if (c == '\n') {
                outtextxy(150, y, line.c_str());
                line.clear();
                y += 30;
            }
            else {
                line += c;
            }
        }
        outtextxy(150, y, line.c_str());

        settextstyle(18, 0, "宋体");
        settextcolor(DARKGRAY);
        outtextxy(300, 560, "点击任意位置重新开始游戏");
    }
}

// ===================== 鼠标点击处理 =====================
void HandleMouseClick(int x, int y) {
    // 结局场景点击重新开始
    if (game.currentScene == SCENE_ENDING) {
        RestartGame();
        return;
    }

    // 考场场景点击处理
    if (game.currentScene == SCENE_EXAM) {
        for (int i = 0; i < 4; i++) {
            bool isClick = (x >= areaOption[i].x && x <= areaOption[i].x + areaOption[i].width && y >= areaOption[i].y && y <= areaOption[i].y + areaOption[i].height && areaOption[i].isEnable);
            if (isClick) {
                // 答对处理
                if (i == game.currentQuestion.correctIndex) {
                    int addScore = game.day == 30 ? 20 : 10;
                    game.subjectScore[game.currentQuestion.subject] += addScore;
                    game.subjectScore[game.currentQuestion.subject] = min(150, game.subjectScore[game.currentQuestion.subject]);
                    UpdateTotalScore();
                    game.eventText = "答题正确！+" + to_string(addScore) + "分";
                }
                // 答错处理
                else {
                    game.eventText = "答错了！本次答题无加分";
                }

                // 重置状态，切换场景
                game.currentScene = SCENE_CLASSROOM;
                game.answerTimeLeft = 0;
                game.day++;
                ResetAllHoverState();
                return;
            }
        }
        return;
    }

    // 教室场景处理
    if (game.currentScene == SCENE_CLASSROOM) {
        if (game.isPlayingHideGame) {
            // 躲班主任游戏点击
            bool clickPhone = (x >= areaPhone.x && x <= areaPhone.x + areaPhone.width && y >= areaPhone.y && y <= areaPhone.y + areaPhone.height);
            if (clickPhone) {
                if (game.teacherIsComing) {
                    game.eventText = "成功躲过班主任！疲劳-30";
                    game.fatigue -= 30;
                }
                else {
                    game.eventText = "虚惊一场，班主任没来，继续摸鱼！疲劳-20";
                    game.fatigue -= 20;
                }
                game.fatigue = max(0, game.fatigue);
                game.isPlayingHideGame = false;
                game.day++;
            }
            return;
        }

        // 点击课本刷题
        if (areaBook.isHover) {
            GenerateRandomQuestion();
            game.currentScene = SCENE_EXAM;
            game.answerTimeLeft = 15;
            game.eventText = "开始刷题！15秒内答对题目加分";
            game.fatigue += 20;
            ResetAllHoverState();
        }
        // 点击黑板听课
        else if (areaBlackboard.isHover) {
            for (int i = 0; i < SUBJECT_COUNT; i++) {
                game.subjectScore[i] += 5;
                game.subjectScore[i] = min(150, game.subjectScore[i]);
            }
            UpdateTotalScore();
            game.fatigue += 10;
            game.eventText = "认真听完了一节课，全科目+5分！";
            game.day++;
        }
        // 点击手机摸鱼
        else if (areaPhone.isHover) {
            if (rand() % 10 < 5) {
                game.isPlayingHideGame = true;
                game.hideGameTimer = 3;
                game.teacherIsComing = (rand() % 10 < 5);
                game.eventText = "班主任正在后门巡逻！3秒内再次点击手机藏起来！";
            }
            else {
                game.fatigue -= 30;
                game.fatigue = max(0, game.fatigue);
                game.eventText = "偷偷玩了会儿手机，精神多了！疲劳-30";
                game.day++;
            }
        }

        // 疲劳值满100强制休息
        if (game.fatigue >= 100) {
            game.fatigue = 50;
            game.eventText = "你太累了，直接趴在课桌上睡了一整天！";
            game.day++;
        }

        // 随机事件
        if (rand() % 10 < 3 && !game.isPlayingHideGame) {
            RandomEvent();
        }

        // 月考触发
        if (game.day == 10 || game.day == 20) {
            StartMonthlyExam();
        }

        // 高考触发
        if (game.day > 30) {
            StartFinalExam();
        }

        // 数值限制
        game.totalScore = max(0, min(750, game.totalScore));
        game.fatigue = max(0, min(100, game.fatigue));
    }

    // 志愿填报场景处理
    else if (game.currentScene == SCENE_VOLUNTEER) {
        for (int i = 0; i < 7; i++) {
            if (areaVolunteer[i].isHover && areaVolunteer[i].isEnable) {
                game.volunteerChoice = i;
                GenerateResult();
                game.currentScene = SCENE_ENDING;
                return;
            }
        }
    }
}

// ===================== 随机事件 =====================
void RandomEvent() {
    int eventId = rand() % 11;
    switch (eventId) {
    case 0:
        game.eventText = "体育老师又生病了，这节课上数学！\n数学+15分，疲劳+15";
        game.subjectScore[MATH] += 15;
        game.fatigue += 15;
        break;
    case 1:
        game.eventText = "同桌偷偷塞给你一份学霸笔记！\n全学科+10分";
        for (int i = 0; i < SUBJECT_COUNT; i++) game.subjectScore[i] += 10;
        break;
    case 2:
        game.eventText = "班主任在后门窗户偷看你！\n被抓现行，批评教育\n疲劳+20";
        game.fatigue += 20;
        break;
    case 3:
        game.eventText = "李华又让你帮他写英语作文！\n英语+15分，疲劳+10";
        game.subjectScore[ENGLISH] += 15;
        game.fatigue += 10;
        break;
    case 4:
        game.eventText = "考试前一天你发烧了！\n发挥失常，总分-20分";
        game.totalScore -= 20;
        break;
    case 5:
        game.eventText = "老师：\"这道题我讲过多少遍了！\"\n你突然就懂了，全科目+5分";
        for (int i = 0; i < SUBJECT_COUNT; i++) game.subjectScore[i] += 5;
        break;
    case 6:
        game.eventText = "妈妈给你炖了爱心鸡汤！\n疲劳-20";
        game.fatigue -= 20;
        break;
    case 7:
        game.eventText = "上课睡觉被老师点名回答问题！\n尴尬到抠脚，疲劳+15";
        game.fatigue += 15;
        break;
    case 8:
        game.eventText = "模考中超常发挥！\n总分+25分";
        game.totalScore += 25;
        break;
    case 9:
        game.eventText = "和同桌吵架了，心情很差\n疲劳+25，总分-10分";
        game.fatigue += 25;
        game.totalScore -= 10;
        break;
    case 10:
        game.eventText = "参加学科竞赛获得一等奖！\n对应科目+30分";
        game.subjectScore[rand() % SUBJECT_COUNT] += 30;
        break;
    }
    UpdateTotalScore();
    game.fatigue = max(0, min(100, game.fatigue));
}

// ===================== 工具函数 =====================
void GenerateRandomQuestion() {
    int index = rand() % 12;
    game.currentQuestion = questionBank[index];
}

void StartMonthlyExam() {
    GenerateRandomQuestion();
    game.currentScene = SCENE_EXAM;
    game.answerTimeLeft = 20;
    game.eventText = "月考开始了！20秒内答对题目获得大量加分";
    ResetAllHoverState();
}

void StartFinalExam() {
    GenerateRandomQuestion();
    game.currentScene = SCENE_EXAM;
    game.answerTimeLeft = 30;
    game.eventText = "高考最终场！30秒内答对题目决定你的最终分数";
    ResetAllHoverState();
}

void UpdateTotalScore() {
    game.totalScore = 0;
    for (int i = 0; i < SUBJECT_COUNT; i++) {
        game.subjectScore[i] = min(150, game.subjectScore[i]);
        game.totalScore += game.subjectScore[i];
    }
    game.totalScore = max(0, min(750, game.totalScore));
}

void GenerateResult() {
    int score = game.totalScore;
    int choice = game.volunteerChoice;

    if (choice == 6) {
        game.resultText = "你决定复读一年！\n\n"
            "这一年你戒掉了手机，发奋图强，\n"
            "终于在第二年考上了理想的大学！\n\n"
            "（点击重新开始，体验你的复读生涯）";
        return;
    }

    bool isSlip = false;
    if (choice == 0 && score < 700) isSlip = true;
    else if (choice == 1 && score < 620) isSlip = true;
    else if (choice == 2 && score < 570) isSlip = true;
    else if (choice == 3 && score < 520) isSlip = true;
    else if (choice == 4 && score < 420) isSlip = true;
    else if (choice == 5 && score < 220) isSlip = true;

    if (isSlip) {
        if (choice == 0) choice = 1;
        else if (choice == 1) choice = 2;
        else if (choice == 2) choice = 3;
        else if (choice == 3) choice = 4;
        else if (choice == 4) choice = 5;
        else {
            game.resultText = "很遗憾，你填报的所有志愿都滑档了！\n\n"
                "你决定先去社会上闯荡，\n"
                "几年后创办了自己的科技公司，\n"
                "成为了身价过亿的企业家！";
            return;
        }
        game.resultText = "⚠️ 第一志愿滑档了！你被调剂到了：\n\n";
    }
    else {
        game.resultText = "🎉 恭喜你！成功被录取！\n\n录取院校：\n\n";
    }

    switch (choice) {
    case 0:
        game.resultText += score >= 730 ? "清华大学（全省理科状元）\n\n你成为了全省高考状元，\n各大媒体争相报道，家乡为你立了牌坊！\n毕业后进入中科院，成为顶尖科学家。"
            : "北京大学\n\n你在燕园度过了美好的四年，\n毕业后进入华尔街顶级投行，\n年薪百万，实现了财务自由。";
        break;
    case 1:
        if (score >= 670) game.resultText += "复旦大学\n\n你在魔都上海开启了新人生，\n毕业后进入字节跳动，\n成为了一名优秀的算法工程师。";
        else if (score >= 640) game.resultText += "浙江大学\n\n你在美丽的杭州学习计算机，\n毕业后进入阿里巴巴，\n参与了双十一核心系统的开发。";
        else game.resultText += "某985工程大学\n\n你在大学里成绩优异，获得保研资格，\n继续攻读硕士博士学位，\n毕业后留校任教，成为一名大学老师。";
        break;
    case 2:
        if (score >= 590) game.resultText += "北京邮电大学\n\n你学习了通信工程专业，\n毕业后进入华为公司，\n被派往海外参与5G基站建设。";
        else if (score >= 570) game.resultText += "中国政法大学\n\n你学习了法学专业，\n毕业后通过了司法考试，\n成为了一名知名律师。";
        else game.resultText += "某211工程大学\n\n你在大学里担任学生会主席，\n锻炼了出色的组织能力，\n毕业后考上了公务员。";
        break;
    case 3:
        if (score >= 540) game.resultText += "某医科大学\n\n你学习了临床医学专业，\n毕业后进入三甲医院，\n成为了一名救死扶伤的医生。";
        else if (score >= 520) game.resultText += "某师范大学\n\n你学习了师范专业，\n毕业后回到家乡成为一名高中老师，\n培养出了很多优秀的学生。";
        else game.resultText += "某普通一本大学\n\n你在大学里谈了一场甜甜的恋爱，\n毕业后和对象一起去了深圳打拼，\n虽然辛苦但很幸福。";
        break;
    case 4:
        if (score >= 470) game.resultText += "某艺术学院\n\n你学习了视觉传达设计专业，\n毕业后成为了一名自由设计师，\n工作时间自由，收入也不错。";
        else if (score >= 430) game.resultText += "某三本大学\n\n你在大学里就开始创业，\n开了一家校园外卖平台，\n毕业时已经月入过万。";
        else game.resultText += "某民办本科大学\n\n你在大学里努力学习专业技能，\n毕业后进入了一家互联网公司，\n从基层做起，一步步晋升为部门经理。";
        break;
    case 5:
        if (score >= 350) game.resultText += "某师范专科学校\n\n你学习了学前教育专业，\n毕业后成为了一名幼儿园老师，\n每天和可爱的孩子们在一起。";
        else if (score >= 280) game.resultText += "某职业技术学院\n\n你学习了烹饪专业，\n毕业后成为了一名大厨，\n后来开了自己的餐厅，生意火爆。";
        else game.resultText += "蓝翔技工学校\n\n你学习了挖掘机专业，\n技术精湛，人称\"挖掘机小王子\"，\n月薪过万，比很多大学生赚得都多！";
        break;
    }

    if (isSlip) {
        game.resultText += "\n\n虽然第一志愿没考上，\n但这也是一个不错的结果！\n是金子在哪里都会发光的✨";
    }
}

void RestartGame() {
    InitGame();
}
