#include "menu.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

menu::menu() {}

int menu::exibirMenu() {
    int escolha;
    cout << "1. Iniciar Jogo" << endl;
    cout << "2. Configurar Jogo" << endl;
    cout << "3. Ver Histórico de Partidas" << endl;  // Nova opção
    cout << "4. Sair" << endl;
    cout << "Escolha uma opcao: ";
    cin >> escolha;
    return escolha;
}

// Função para alterar configurações do jogo
void menu::configurarJogo(int& roundTime, int& scoreLimit) {
    cout << "===== Configuracoes =====" << endl;
    cout << "Tempo por rodada (segundos): ";
    cin >> roundTime;
    cout << "Pontuacao limite: ";
    cin >> scoreLimit;
    cout << "Configuracoes atualizadas!" << endl;
}

void menu::exibirHistorico() {
    ifstream historico("historico_partidas.txt"); 
    vector<string> historicov;
    if (historico.is_open()) {
        string linha;
        // Lê o arquivo linha por linha e adiciona ao vetor
        while (getline(historico, linha)) {
            historicov.push_back(linha);
        }


        historico.close();
    }
    else {
        cerr << "Erro ao abrir o arquivo de histórico!" << endl;
        return;
    }
    if (historicov.empty()) {
        cout << "Nenhuma partida registrada no histórico." << endl;
    }
    else {
        cout << "Histórico de partidas:" << endl;
        for (const string& partida : historicov) {
            cout << partida << endl;
        }
    }
}


  

  