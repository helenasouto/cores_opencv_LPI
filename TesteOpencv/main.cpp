#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <vector>
#include <fstream>  
#include "menu.h"

using namespace cv;
using namespace std;

int roundTime = 30;
int scoreLimit = 10;
menu menuP;

void salvarHistorico(const string& nomeJogador1, int pontuacaoJogador1, const string& nomeJogador2, int pontuacaoJogador2) {
    ofstream historico("historico_partidas.txt", ios::app); // Abre o arquivo no modo de adição
    if (historico.is_open()) {
        historico << "Partida:\n";
        historico << nomeJogador1 << " - Pontuação: " << pontuacaoJogador1 << "\n";
        historico << nomeJogador2 << " - Pontuação: " << pontuacaoJogador2 << "\n";
        historico << "------------------------------------\n";
        historico.close();
    }
    else {
        cerr << "Erro ao abrir o arquivo de histórico!" << endl;
    }
}

int main() {
    string nomeJogador1, nomeJogador2;

    // Coletar nomes dos jogadores
    cout << "Digite o nome do Jogador 1: ";
    getline(cin, nomeJogador1);
    cout << "Digite o nome do Jogador 2: ";
    getline(cin, nomeJogador2);

    while (true) {
        int escolha = menuP.exibirMenu();

        if (escolha == 1) {
            // Inicializa a captura da câmera
            VideoCapture cap(0);
            if (!cap.isOpened()) {
                cerr << "Erro ao abrir a câmera!" << endl;
                return -1;
            }

            // Define a resolução do frame para 1920x1080
            cap.set(CAP_PROP_FRAME_WIDTH, 1920);
            cap.set(CAP_PROP_FRAME_HEIGHT, 1080);

            // Ajustes das cores a serem detectadas
            Scalar redLower1(0, 120, 120), redUpper1(10, 255, 255);
            Scalar redLower2(170, 120, 120), redUpper2(180, 255, 255);
            Scalar greenLower(35, 100, 100), greenUpper(85, 255, 255);
            Scalar blueLower(90, 100, 100), blueUpper(130, 255, 255);
            Scalar yellowLower(20, 100, 100), yellowUpper(30, 255, 255);
            Scalar orangeLower(10, 100, 100), orangeUpper(25, 255, 255);  // Adicionando laranja
            Scalar purpleLower(130, 100, 100), purpleUpper(160, 255, 255); // Adicionando roxo

            // Lista de nomes das cores e seus respectivos limites
            vector<string> colorNames = { "Vermelho", "Verde", "Azul", "Amarelo", "Laranja", "Roxo" };
            vector<pair<Scalar, Scalar>> colorBounds = {
                { redLower1, redUpper1 }, { redLower2, redUpper2 },
                { greenLower, greenUpper }, { blueLower, blueUpper },
                { yellowLower, yellowUpper }, { orangeLower, orangeUpper },
                { purpleLower, purpleUpper }
            };

            // Variáveis para contagem e tempo de detecção
            int player1Score = 0;
            int player2Score = 0;
            int currentPlayer = 1; // 1 para jogador 1, 2 para jogador 2
            int requiredDetectionTime = 3;
            bool gameActive = true;
            bool turnMessageDisplayed = true; // Variável para exibir mensagem de troca de jogador

            // Inicializando o temporizador e a lógica de sorteio
            srand(time(0));
            int targetColorIndex = rand() % 6;  // Agora sorteamos entre 6 cores (incluindo laranja e roxo)
            string targetColor = colorNames[targetColorIndex];
            int previousColorIndex = -1;  // Variável para armazenar a cor anterior

            // Variáveis de controle
            auto roundStartTime = chrono::steady_clock::now();
            auto colorStartTime = chrono::steady_clock::time_point();
            bool colorDetected = false;
            bool showMessage = false;
            string message = "";

            // Função para escurecer a imagem
            auto darkenFrame = [](Mat& frame) {
                Mat mask(frame.size(), frame.type(), Scalar(0, 0, 0));
                addWeighted(frame, 0.3, mask, 0.7, 0, frame);
                };

            // Função para desenhar texto com fundo escuro
            auto drawTextWithBackground = [](Mat& img, const string& text, Point org, int fontFace, double fontScale, Scalar textColor, int thickness) {
                int baseline = 0;
                Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
                baseline += thickness;

                Rect bgRect(org.x, org.y - textSize.height, textSize.width, textSize.height + baseline);
                rectangle(img, bgRect, Scalar(0, 0, 0), FILLED);

                putText(img, text, org, fontFace, fontScale, textColor, thickness);
                };

            while (gameActive) {
                Mat frame;
                cap >> frame;

                if (frame.empty()) break;

                flip(frame, frame, 1);

                GaussianBlur(frame, frame, Size(5, 5), 0);
                Mat hsv;
                cvtColor(frame, hsv, COLOR_BGR2HSV);

                // Verifica se é necessário exibir a mensagem de turno do jogador
                if (turnMessageDisplayed) {
                    darkenFrame(frame);
                    string turnMessage = "Vez do Jogador " + to_string(currentPlayer);
                    drawTextWithBackground(frame, turnMessage, Point(frame.cols / 4, frame.rows / 2), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 3);

                    auto currentTime = chrono::steady_clock::now();
                    auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - roundStartTime).count();
                    if (elapsedTime >= 3) {  // Exibir a mensagem por 3 segundos
                        turnMessageDisplayed = false;
                        roundStartTime = chrono::steady_clock::now();  // Reinicia o temporizador da rodada
                        PlaySound(TEXT("som2.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);  // Inicia som de cronômetro
                    }

                    imshow("Jogo de Deteccao de Cores", frame);
                    if (waitKey(30) == 's') break;
                    continue;  // Pula o resto do loop até que a mensagem seja exibida
                }

                // Máscara para a cor sorteada
                Mat mask;
                if (targetColor == "Vermelho") {
                    Mat mask1, mask2;
                    inRange(hsv, redLower1, redUpper1, mask1);
                    inRange(hsv, redLower2, redUpper2, mask2);
                    mask = mask1 | mask2;
                }
                else if (targetColor == "Verde") {
                    inRange(hsv, greenLower, greenUpper, mask);
                }
                else if (targetColor == "Azul") {
                    inRange(hsv, blueLower, blueUpper, mask);
                }
                else if (targetColor == "Amarelo") {
                    inRange(hsv, yellowLower, yellowUpper, mask);
                }
                else if (targetColor == "Laranja") {
                    inRange(hsv, orangeLower, orangeUpper, mask);  // Detecção de cor laranja
                }
                else if (targetColor == "Roxo") {
                    inRange(hsv, purpleLower, purpleUpper, mask);  // Detecção de cor roxo
                }

                // Define o quadrado no centro da tela
                int squareSize = 200;
                int x = (frame.cols - squareSize) / 2;
                int y = (frame.rows - squareSize) / 2;
                Rect square(x, y, squareSize, squareSize);

                // Mapeia o nome da cor sorteada para o valor Scalar correspondente
                Scalar squareColor;
                if (targetColor == "Vermelho") {
                    squareColor = Scalar(0, 0, 255);  // Vermelho
                }
                else if (targetColor == "Verde") {
                    squareColor = Scalar(0, 255, 0);  // Verde
                }
                else if (targetColor == "Azul") {
                    squareColor = Scalar(255, 0, 0);  // Azul
                }
                else if (targetColor == "Amarelo") {
                    squareColor = Scalar(0, 255, 255);  // Amarelo
                }
                else if (targetColor == "Laranja") {
                    squareColor = Scalar(0, 165, 255);  // Laranja
                }
                else if (targetColor == "Roxo") {
                    squareColor = Scalar(255, 0, 255);  // Roxo
                }
                else {
                    squareColor = Scalar(255, 255, 255);  // Cor padrão, se nenhuma cor for encontrada
                }

                // Desenha o quadrado com a cor correspondente
                rectangle(frame, square, squareColor, 2);

                if (!showMessage) {
                    Mat maskROI = mask(square);
                    if (countNonZero(maskROI) > (squareSize * squareSize) * 0.1) {
                        if (!colorDetected) {
                            colorDetected = true;
                            colorStartTime = chrono::steady_clock::now();
                        }
                        else {
                            auto currentTime = chrono::steady_clock::now();
                            auto elapsedColorTime = chrono::duration_cast<chrono::seconds>(currentTime - colorStartTime).count();
                            int remainingColorTime = requiredDetectionTime - elapsedColorTime;
                            drawTextWithBackground(frame, "Detectando " + targetColor + ": " + to_string(remainingColorTime) + "s", Point(10, y - 40), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

                            if (elapsedColorTime >= requiredDetectionTime) {
                                if (currentPlayer == 1) {
                                    player1Score++;
                                }
                                else {
                                    player2Score++;
                                }
                                message = "Jogador " + to_string(currentPlayer) + " acertou! Pontos: " + to_string(currentPlayer == 1 ? player1Score : player2Score);
                                showMessage = true;
                                PlaySound(TEXT("som1.wav"), NULL, SND_FILENAME | SND_ASYNC);

                                roundStartTime = chrono::steady_clock::now();
                            }
                        }
                    }
                    else {
                        colorDetected = false;
                    }

                    drawTextWithBackground(frame, "Traga a cor: " + targetColor, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255, 255, 255), 2);
                    drawTextWithBackground(frame, "Pontos Jogador 1: " + to_string(player1Score), Point(10, 80), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
                    drawTextWithBackground(frame, "Pontos Jogador 2: " + to_string(player2Score), Point(10, 110), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
                    drawTextWithBackground(frame, "Jogador Atual: " + to_string(currentPlayer), Point(10, 140), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

                    auto currentTime = chrono::steady_clock::now();
                    auto elapsedRoundTime = chrono::duration_cast<chrono::seconds>(currentTime - roundStartTime).count();
                    int remainingTime = roundTime - elapsedRoundTime;
                    drawTextWithBackground(frame, "Tempo restante: " + to_string(remainingTime) + "s", Point(10, 170), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

                    if (remainingTime <= 0) {
                        message = "Tempo esgotado! Jogador " + to_string(currentPlayer) + " perdeu a rodada.";
                        showMessage = true;
                        roundStartTime = chrono::steady_clock::now();
                    }
                }

                if (showMessage) {
                    darkenFrame(frame);
                    drawTextWithBackground(frame, message, Point(frame.cols / 4, frame.rows / 2), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 3);

                    auto currentTime = chrono::steady_clock::now();
                    auto elapsedMessageTime = chrono::duration_cast<chrono::seconds>(currentTime - roundStartTime).count();

                    // Certifique-se de que a mensagem é exibida por pelo menos 3 segundos
                    if (elapsedMessageTime >= 3) {
                        if (player1Score >= scoreLimit || player2Score >= scoreLimit) {
                            // Aqui entra o código corrigido
                            message = "Jogador " + to_string(player1Score >= scoreLimit ? 1 : 2) + " venceu!";
                            PlaySound(TEXT("som3.wav"), NULL, SND_FILENAME | SND_ASYNC);
                            darkenFrame(frame);
                            drawTextWithBackground(frame, message, Point(frame.cols / 4, frame.rows / 2), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 255, 0), 3);

                            // Exibe a janela com a mensagem de vitória
                            imshow("Jogo de Deteccao de Cores", frame);

                            // Espera por alguns segundos para garantir que a mensagem de vitória seja visível
                            waitKey(3000);  // Espera 3 segundos

                            salvarHistorico(nomeJogador1, player1Score, nomeJogador2, player2Score);  // Salva o histórico da partida
                            cout << "Histórico da partida salvo!" << endl;
                            break;  // Encerra o jogo após a exibição da mensagem final
                        }

                        // Reinicia o turno após exibir a mensagem por 3 segundos
                        do {
                            targetColorIndex = rand() % 6;  // Agora sorteia entre 7 cores (incluindo laranja e roxo)
                        } while (targetColorIndex == previousColorIndex);

                        targetColor = colorNames[targetColorIndex];
                        previousColorIndex = targetColorIndex;  // Atualiza a cor anterior

                        currentPlayer = (currentPlayer == 1) ? 2 : 1;  // Alterna entre os jogadores
                        turnMessageDisplayed = true;  // Define para exibir a mensagem de vez do jogador
                        roundStartTime = chrono::steady_clock::now();  // Reinicia o cronômetro
                        showMessage = false;  // Finaliza a exibição da mensagem
                    }
                }


                imshow("Jogo de Deteccao de Cores", frame);
                if (waitKey(30) == 's') break;
            }

            cap.release();
            cv::destroyAllWindows();
        }
        else if (escolha == 2) {
            menuP.configurarJogo(roundTime, scoreLimit); 
        }
        else if (escolha == 3) {
            menuP.exibirHistorico();  
        }
        else if (escolha == 4 or escolha == 's') {
            break;
        }
        else {
            cout << "Opção Inválida!";
            break;
        }
    }

    return 0;
}