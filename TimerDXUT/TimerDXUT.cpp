/**********************************************************************************
// Pong - Computação Gráfica
// Integrantes do Grupo: Fábio Vinícius, Renato Rocha, Letícia Manuelle
**********************************************************************************/

#include "DXUT.h"
#include <string>

// Estados da Máquina de Estados
enum GameState {
    MENU,
    PLAYING,
    GAMEOVER
};

class WinApp : public App
{
private:
    GameState currentState;

    // Configurações da Partida
    bool vsCPU;
    int maxScore;
    int winner;

    // Entidades do jogo
    float p1X, p1Y;
    float p2X, p2Y;
    float paddleWidth, paddleHeight;
    float paddleSpeed;

    float ballX, ballY;
    float ballSize;
    float ballVelX, ballVelY;

    int scoreP1;
    int scoreP2;

    // Funções auxiliares
    void DrawRectangle(HDC hDC, int x, int y, int width, int height);
    void ResetBall(bool p1Scored);
    void ResetGame();

public:
    void Init();
    void Update();
    void Draw();
    void Finalize();
};

// ------------------------------------------------------------------------------

void WinApp::Init()
{
    currentState = MENU;
    vsCPU = true;
    maxScore = 5;

    paddleWidth = 15.0f;
    paddleHeight = 100.0f;
    ballSize = 15.0f;
    paddleSpeed = 400.0f;
}

// ------------------------------------------------------------------------------

void WinApp::ResetGame()
{
    scoreP1 = 0;
    scoreP2 = 0;
    winner = 0;

    p1X = 30.0f;
    p1Y = (window->Height() - paddleHeight) / 2.0f;

    p2X = window->Width() - 30.0f - paddleWidth;
    p2Y = (window->Height() - paddleHeight) / 2.0f;

    ResetBall(true);
}

// ------------------------------------------------------------------------------

void WinApp::ResetBall(bool p1Scored)
{
    ballX = (window->Width() - ballSize) / 2.0f;
    ballY = (window->Height() - ballSize) / 2.0f;

    ballVelX = p1Scored ? 350.0f : -350.0f;
    ballVelY = 200.0f;
}

// ------------------------------------------------------------------------------

void WinApp::DrawRectangle(HDC hDC, int x, int y, int width, int height)
{
    MoveToEx(hDC, x, y, NULL);
    LineTo(hDC, x + width, y);
    LineTo(hDC, x + width, y + height);
    LineTo(hDC, x, y + height);
    LineTo(hDC, x, y);
}

// ------------------------------------------------------------------------------

void WinApp::Update()
{
    // Sai do jogo a qualquer momento com ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    float dt = frameTime;

    if (currentState == MENU)
    {
        if (input->KeyPress('M')) vsCPU = !vsCPU;
        if (input->KeyPress(VK_UP)) maxScore++;
        if (input->KeyPress(VK_DOWN) && maxScore > 1) maxScore--;

        if (input->KeyPress(VK_RETURN))
        {
            ResetGame();
            currentState = PLAYING;
        }
    }
    else if (currentState == PLAYING)
    {
        // Controles Jogador 1 (W/S)
        if (input->KeyDown('W')) p1Y -= paddleSpeed * dt;
        if (input->KeyDown('S')) p1Y += paddleSpeed * dt;

        if (p1Y < 0) p1Y = 0;
        if (p1Y + paddleHeight > window->Height()) p1Y = window->Height() - paddleHeight;

        // Controles Jogador 2 (IA ou Setas)
        if (vsCPU)
        {
            float centerP2 = p2Y + (paddleHeight / 2.0f);
            float ballCenterY = ballY + (ballSize / 2.0f);
            float cpuSpeed = paddleSpeed * 0.85f;

            if (centerP2 < ballCenterY - 10.0f) p2Y += cpuSpeed * dt;
            else if (centerP2 > ballCenterY + 10.0f) p2Y -= cpuSpeed * dt;
        }
        else
        {
            if (input->KeyDown(VK_UP)) p2Y -= paddleSpeed * dt;
            if (input->KeyDown(VK_DOWN)) p2Y += paddleSpeed * dt;
        }

        if (p2Y < 0) p2Y = 0;
        if (p2Y + paddleHeight > window->Height()) p2Y = window->Height() - paddleHeight;

        // Movimentação da bola
        ballX += ballVelX * dt;
        ballY += ballVelY * dt;

        // Colisão com teto e chão
        if (ballY <= 0) { ballY = 0; ballVelY = -ballVelY; }
        else if (ballY + ballSize >= window->Height()) { ballY = window->Height() - ballSize; ballVelY = -ballVelY; }

        // Colisão com rebatedor 1 (Esquerda)
        if (ballX <= p1X + paddleWidth && ballX + ballSize >= p1X && ballY + ballSize >= p1Y && ballY <= p1Y + paddleHeight)
        {
            if (ballVelX < 0) { ballX = p1X + paddleWidth; ballVelX = -ballVelX * 1.05f; }
        }

        // Colisão com rebatedor 2 (Direita)
        if (ballX + ballSize >= p2X && ballX <= p2X + paddleWidth && ballY + ballSize >= p2Y && ballY <= p2Y + paddleHeight)
        {
            if (ballVelX > 0) { ballX = p2X - ballSize; ballVelX = -ballVelX * 1.05f; }
        }

        // Sistema de pontuação
        if (ballX + ballSize < 0)
        {
            scoreP2++;
            if (scoreP2 >= maxScore) { winner = 2; currentState = GAMEOVER; }
            else ResetBall(false);
        }
        else if (ballX > window->Width())
        {
            scoreP1++;
            if (scoreP1 >= maxScore) { winner = 1; currentState = GAMEOVER; }
            else ResetBall(true);
        }
    }
    else if (currentState == GAMEOVER)
    {
        if (input->KeyPress(VK_RETURN)) currentState = MENU;
    }
}

// ------------------------------------------------------------------------------

void WinApp::Draw()
{
    HDC hDC = GetDC(window->Id());

    if (hDC)
    {
        RECT rect;
        GetClientRect(window->Id(), &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Configuração do Buffer Duplo para evitar Flickering
        HDC hMemDC = CreateCompatibleDC(hDC);
        HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, width, height);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);

        // Fundo da tela
        HBRUSH bgBrush = CreateSolidBrush(RGB(0, 122, 204));
        FillRect(hMemDC, &rect, bgBrush);
        DeleteObject(bgBrush);

        SetBkMode(hMemDC, TRANSPARENT);
        SetTextColor(hMemDC, RGB(255, 255, 255));

        int centerX = window->Width() / 2;
        int centerY = window->Height() / 2;

        // Renderização baseada no estado atual
        if (currentState == MENU)
        {
            std::string title = "=== PONG GAME ===";
            std::string modeText = vsCPU ? "Modo: [ Jogador vs Computador ] (Pressione 'M' para trocar)"
                : "Modo: [ Jogador vs Jogador ] (Pressione 'M' para trocar)";
            std::string scoreText = "Limite de Pontos: " + std::to_string(maxScore) + " (Setas Cima/Baixo para alterar)";
            std::string startText = "Pressione [ ENTER ] para Iniciar";
            std::string exitText = "Pressione [ ESC ] para Sair";

            TextOutA(hMemDC, centerX - 60, centerY - 80, title.c_str(), title.length());
            TextOutA(hMemDC, centerX - 180, centerY - 20, modeText.c_str(), modeText.length());
            TextOutA(hMemDC, centerX - 150, centerY + 10, scoreText.c_str(), scoreText.length());
            TextOutA(hMemDC, centerX - 100, centerY + 60, startText.c_str(), startText.length());
            TextOutA(hMemDC, centerX - 80, centerY + 90, exitText.c_str(), exitText.length());
        }
        else if (currentState == PLAYING)
        {
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            HPEN hOldPen = (HPEN)SelectObject(hMemDC, hPen);

            DrawRectangle(hMemDC, (int)p1X, (int)p1Y, (int)paddleWidth, (int)paddleHeight);
            DrawRectangle(hMemDC, (int)p2X, (int)p2Y, (int)paddleWidth, (int)paddleHeight);
            DrawRectangle(hMemDC, (int)ballX, (int)ballY, (int)ballSize, (int)ballSize);

            // Linha divisória da quadra
            for (int y = 0; y < window->Height(); y += 20)
            {
                MoveToEx(hMemDC, centerX, y, NULL);
                LineTo(hMemDC, centerX, y + 10);
            }

            std::string p1Text = "JOGADOR 1: " + std::to_string(scoreP1);
            std::string p2Text = (vsCPU ? "CPU: " : "JOGADOR 2: ") + std::to_string(scoreP2);

            TextOutA(hMemDC, centerX - 150, 20, p1Text.c_str(), p1Text.length());
            TextOutA(hMemDC, centerX + 50, 20, p2Text.c_str(), p2Text.length());

            SelectObject(hMemDC, hOldPen);
            DeleteObject(hPen);
        }
        else if (currentState == GAMEOVER)
        {
            std::string overTitle = "=== FIM DE JOGO ===";
            std::string winText = "VENCEDOR: ";
            if (winner == 1) winText += "JOGADOR 1!";
            else winText += (vsCPU ? "COMPUTADOR!" : "JOGADOR 2!");

            std::string restartText = "Pressione [ ENTER ] para voltar ao Menu";

            TextOutA(hMemDC, centerX - 60, centerY - 40, overTitle.c_str(), overTitle.length());
            TextOutA(hMemDC, centerX - 80, centerY, winText.c_str(), winText.length());
            TextOutA(hMemDC, centerX - 120, centerY + 50, restartText.c_str(), restartText.length());
        }

        // Transfere o buffer da memória para a tela
        BitBlt(hDC, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

        // Limpeza dos objetos do buffer
        SelectObject(hMemDC, hOldBmp);
        DeleteObject(hMemBmp);
        DeleteDC(hMemDC);

        ReleaseDC(window->Id(), hDC);
    }
}

void WinApp::Finalize() {}

// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    Engine* engine = new Engine();
    engine->window->Mode(WINDOWED);
    engine->window->Size(1024, 600);
    engine->window->Color(0, 122, 204);
    engine->window->Title("Pong");
    engine->window->Icon(IDI_ICON);
    engine->window->Cursor(IDC_CURSOR);

    int exitCode = engine->Start(new WinApp());

    delete engine;
    return exitCode;
}