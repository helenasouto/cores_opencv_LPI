#ifndef MENU_H
#define MENU_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class menu {

public:
	menu();
	int escolha;
	int exibirMenu();
	void configurarJogo(int& roundTime, int& scoreLimit);
	void exibirHistorico();
};

#endif