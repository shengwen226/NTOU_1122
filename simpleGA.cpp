#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <fstream>
#include <direct.h>
#include <ctime>
using namespace std;

int main(){
	/* 讀取檔案 */
	string goods_filename = ".\\GA_input\\goods.dat";
	ifstream read_goods(goods_filename.c_str());
	if (!read_goods) {
		cout << "Error: Failed to open " << goods_filename << endl;
		system("pause");
		return 0;
	}
	int goods_num;
	read_goods >> goods_num;
	double ** goods = new double * [goods_num];
	for (int i = 0; i < goods_num; i++) {
		goods[i] = new double[5]();
	}
	for (int i = 0; i < goods_num; i++) {
		for (int j = 0; j < 4; j++) {
			read_goods >> goods[i][j];    //goods[num][j]: 0總數量 1重量 2體積 3利潤
		}
	}
	read_goods.close();
	string space_filename = ".\\GA_input\\space.dat";
	ifstream read_space(space_filename.c_str());
	if (!read_space) {
		cout << "Error: Failed to open " << space_filename << endl;
		system("pause");
		return 0;
	}
	int space_num;
	read_space >> space_num;
	double ** space = new double * [space_num];
	for (int i = 0; i < space_num; i++) {
		space[i] = new double[2]();
	}
	for (int i = 0; i < space_num; i++) {
		for (int j = 0; j < 2; j++) {
			read_space >> space[i][j];    //space[num][j]: 0重量容量 1體積容量
		}
	}
	read_space.close();
	int x_num = goods_num*space_num;
	int constraints_num = 3*space_num + goods_num;
	double ** constraints = new double * [constraints_num];    //coefficients in the constraints
	for (int i = 0; i < constraints_num; i++) {
		constraints[i] = new double[x_num]();
	}
	double * RHS = new double[constraints_num]();    //coefficients of the RHS (Right-Hand-Side)
	double * objective = new double[x_num]();    //coefficients of objective function

	/* 建立限制式&目標式矩陣 */
	void create_objective_matrix(int goods_num, double ** goods, int x_num, double * objective);
	void create_constraints_matrix(	int goods_num, double ** goods, int space_num, double ** space,
									int x_num, int constraints_num, double ** constraints, double * RHS);
	create_objective_matrix(goods_num, goods, x_num, objective);
	create_constraints_matrix(goods_num, goods, space_num, space, x_num, constraints_num, constraints, RHS);

	/* Genetic Algorithm */
	// 預設設定
	int population = 1500;			//初始族群個數
	int dimension = x_num;			//變數數量
	int generation = 150;			//演化代數
	int selection_rate = 10;		//選擇率(%)
	int crossover_rate = 40;		//交配率(%)
	int mutation_rate = 60;			//突變率(%)
	int frequency = 0;				//計算次數
	// 讀取設定檔
	string ga_set_filename = ".\\GA.set";
	ifstream read_ga(ga_set_filename.c_str());
	if (!read_ga) {
		cout << "Error: Failed to open " << ga_set_filename << endl;
	}else {
		read_ga >> population >> generation >> selection_rate >> crossover_rate >> mutation_rate >> frequency;
	}
	read_ga.close();
	// 基本
	srand(time(NULL));              //隨機種子
	int ** chromosome = new int * [population];	//染色體
	for (int i = 0; i < population; i++) {
		chromosome[i] = new int[dimension]();
	}
	int * gene_upper = new int[dimension]();	//基因範圍
	for (int i = 0; i < dimension; i++) {
		gene_upper[i] = goods[i % goods_num][0];
	}
	double * fitness = new double[population](); //適應度
	// function
	int random(int lower, int upper);
	void initial_population(int start, int population, int dimension, int ** chromosome, int * gene_upper);
	void calculate_fitness(	int population, int dimension, int ** chromosome, double * fitness,
							double * objective, int constraints_num, double ** constraints, double * RHS);
	int elitism_selection(int selection_rate, int population, int dimension, int ** chromosome, double * fitness);
	int crossover(int start, int mutation_rate, int population, int dimension, int ** chromosome);
	void mutation(int mutation_rate, int population, int dimension, int ** chromosome, int * gene_upper);
	int result(	int population, int dimension, int ** chromosome,
				double * objective,int constraints_num, double ** constraints, double * RHS);
	int obj_value(int dimension, int ** chromosome, double * objective);
	void check(	int obj_ans, int constraints_num, int dimension,int space_num,
				int ** chromosome,double ** constraints, double * objective, double * RHS);
	int start_index, count_gen, count_freq, obj_ans;
	// 輸出檔案
	string out_file_path = ".\\GA_output.dat";
	ofstream outfile(out_file_path.c_str());
	// 開始計算
	cout << "GA -> GO" << endl;
	count_freq = 1;
	clock_t time = clock();
	while (true) {
		// 建立初始族群
		initial_population(0, population, dimension, chromosome, gene_upper);
		count_gen = 1;
		while (true) {
			if (fmod(count_gen*100.0/generation,5) == 0) {
				cout << "\rGA " << count_freq << " -> " << count_gen*100.0/generation << " %  ";
			}
			// 計算適應度
			calculate_fitness(population, dimension, chromosome, fitness, objective, constraints_num, constraints,RHS);
			// 菁英選擇法
			start_index = elitism_selection(selection_rate, population, dimension, chromosome, fitness);
			//cout << i << "\t" << obj_value(dimension, chromosome, objective) << endl;
			if (count_gen == generation) {
				break;
			}
			count_gen ++;
			// 交配
			start_index = crossover(start_index, mutation_rate, population, dimension, chromosome);
			// 補充族群
			initial_population(start_index, population, dimension, chromosome, gene_upper);
			// 突變
			mutation(mutation_rate, population, dimension, chromosome, gene_upper);
		}
		obj_ans = result(population, dimension, chromosome, objective, constraints_num, constraints, RHS);
		if (frequency == 0) {
			if (obj_ans != 0) {
				cout << endl;
				break;
			}
		}else {
			cout << "\r              ";
			cout << "\r" << obj_ans;
			outfile << obj_ans;
			for (int j = 0; j < dimension; j++) {
				cout << "\t" << chromosome[0][j];
				outfile << "\t" << chromosome[0][j];
			}
			cout << endl;
			outfile << endl;
		}
		if (count_freq == frequency) {
			break;
		}
		count_freq ++;
	}
	time = (clock() - time)/CLOCKS_PER_SEC;
	cout << "GA -> END" << endl;
	if (frequency == 0) {
		// 結果
		if (obj_ans != 0) {
			cout << "\nX:\t";
			outfile << obj_ans;
			for (int j = 0; j < dimension; j++) {
				cout << chromosome[0][j] << "\t";
				outfile << "\t" << chromosome[0][j];
			}
			cout << "\nobj:\t" << obj_ans << endl;
			cout << "error:\t" << (obj_ans-13330)/13330.0 <<endl;
			outfile << endl;
		}else {
			cout << "\nFail !" << endl;
		}
	}
	cout << endl;
	cout << "time:\t" << time << endl;
	outfile << "\ntime:\t" << time << endl;
	cout << "pop:\t" << population << endl;
	cout << "gen:\t" << generation << endl;
	cout << "sel:\t" << selection_rate << endl;
	cout << "cro:\t" << crossover_rate << endl;
	cout << "mut:\t" << mutation_rate << endl;
	cout << endl;
	outfile.close();
	//驗算
	if (frequency == 0) {
		check(obj_ans,constraints_num,dimension,space_num,chromosome,constraints,objective,RHS);
	}
	system("pause");
	return 0;
}
void check(	int obj_ans, int constraints_num, int dimension,int space_num,
			int ** chromosome, double ** constraints, double * objective, double * RHS){
	cout << "Check:" << endl;
	double sum;
	for (int i = 0; i < constraints_num; i++) {
		sum = 0;
		for (int j = 0; j < dimension; j++) {
			sum += constraints[i][j]*chromosome[0][j];
		}
		cout << "C" << i << " : " << (int)sum;
		if (i >= 2 * space_num && i < 3 * space_num) {
			cout << " = ";
		}
		else {
			cout << " <= ";
		}
		cout << RHS[i] << endl;
	}
	sum = 0;
	for (int i = 0; i < dimension; i++) {
		sum += chromosome[0][i]*objective[i];
	}
	cout << "Obj : " << sum << " = " << obj_ans << endl;
	cout << endl;
	return;
}
int obj_value(int dimension, int ** chromosome, double * objective){
	int obj_value = 0;
	for (int j = 0; j < dimension; j++) {
		obj_value += chromosome[0][j]*objective[j];
	}
	return obj_value;
}
int result(	int population, int dimension, int ** chromosome,
			double * objective,int constraints_num, double ** constraints, double * RHS){
	int * obj_value = new int[population]();
	int obj_ans;
	double LHS, score;
	for (int i = 0; i < population; i++) {
		score = 0;
		obj_value[i] = 0;
		for (int j = 0; j < constraints_num; j++) {
			LHS = 0;
			for (int k = 0; k < dimension; k++) {
				LHS += chromosome[i][k]*constraints[j][k];
			}
			if (j >= 6 && j <= 8) {
				if (abs(LHS - RHS[j]) >= 0.01) {
					score += 1;
				}
			}else {
				if (LHS > RHS[j]) {
					score += 1;
				}
			}
		}
		if (score == 0) {
			for (int j = 0; j < dimension; j++) {
				obj_value[i] += chromosome[i][j]*objective[j];
			}
			//cout << score << obj_value[i] <<endl;
		}
	}
	for (int i = population - 1; i > 0; i--) {
		for (int j = 0; j < i; j++) {
			if (obj_value[j] < obj_value[j+1]) {
				swap(obj_value[j],obj_value[j+1]);
				for (int k = 0; k < dimension; k++) {
					swap(chromosome[j][k],chromosome[j+1][k]);
				}
			}
		}
	}
	obj_ans = obj_value[0];
	delete [] obj_value;
	return obj_ans;
}
void mutation(int mutation_rate, int population, int dimension, int ** chromosome, int * gene_upper){
	int random(int lower, int upper);
	int num, index;
	for (int i = 0; i < population; i++) {
		if (random(0,100) <= mutation_rate) {
			num = random(1,(dimension - 1)/2);
			for (int j = 1; j <= num; j++) {
				index = random(0,dimension - 1);
				chromosome[i][index] = random(0,gene_upper[index]);
			}
		}
	}
	return;
}
int crossover(int start, int mutation_rate, int population, int dimension, int ** chromosome){
	int random(int lower, int upper);
	int dad, mom, index;
	for (int i = start; i < (population-start)*mutation_rate/100 + start; i++) {
		dad = random(0,start - 1);
		do {
			mom = random(0,start - 1);
		}while (dad == mom);
		for (int j = 0; j < dimension; j++) {
			if (random(0,99) >= 50) {
				chromosome[i][j] = chromosome[dad][j];
			}else {
				chromosome[i][j] = chromosome[mom][j];
			}
		}
		index = i;
	}
	return index + 1;
}
int elitism_selection(int selection_rate, int population, int dimension, int ** chromosome, double * fitness){
	for (int i = population - 1; i > 0; i--) {
		for (int j = 0; j < i; j++) {
			if (fitness[j] < fitness[j+1]) {
				swap(fitness[j],fitness[j+1]);
				for (int k = 0; k < dimension; k++) {
					swap(chromosome[j][k],chromosome[j+1][k]);
				}
			}
		}
	}
	int index = population*selection_rate/100;
	for (int i = index; i < population; i++) {
		fitness[i] = 0;
		for (int j = 0; j < dimension; j++) {
			chromosome[i][j] = 0;
		}
	}
	/*for (int i = 0; i < population; i++) {
		cout << i << "\t" << fitness[i] << endl;
	}*/
	return index;
}
void calculate_fitness(	int population, int dimension, int ** chromosome, double * fitness,
						double * objective, int constraints_num, double ** constraints, double * RHS){
	double ** score = new double * [population];
	for (int i = 0; i < population; i++) {
		score[i] = new double[2]();
	}
	double LHS;
	double s1_min, s1_max, s2_min, s2_max, f_max;
	for (int i = 0; i < population; i++) {
		score[i][0] = 0;
		score[i][1] = 0;
		for (int j = 0; j < constraints_num; j++) {
			LHS = 0;
			for (int k = 0; k < dimension; k++) {
				LHS += chromosome[i][k]*constraints[j][k];
			}
			if (j >= 6 && j <= 8) {
				if (abs(LHS - RHS[j]) <= 10E-3) {
					score[i][0] += 1;
				}
			}else {
				if (LHS <= RHS[j]) {
					score[i][0] += 1;
				}
			}
		}
		for (int k = 0; k < dimension; k++) {
			score[i][1] += chromosome[i][k]*objective[k];
		}
		if (i == 0) {
			s1_min = score[i][0];
			s1_max = score[i][0];
			s2_min = score[i][1];
			s2_max = score[i][1];
		}else {
			if (score[i][0] < s1_min) {
				s1_min = score[i][0];
			}else if (score[i][0] > s1_max) {
				s1_max = score[i][0];
			}
			if (score[i][1] < s2_min) {
				s2_min = score[i][1];
			}else if (score[i][1] > s2_max) {
				s2_max = score[i][1];
			}
		}
	}
	for (int i = 0; i < population; i++) {
		score[i][0] = pow((score[i][0] - s1_min)/(s1_max - s1_min),2);
		score[i][1] = pow((score[i][1] - s2_min)/(s2_max - s2_min),2);
		fitness[i] = score[i][0]*0.9 + score[i][1]*0.1;
	}
	for (int i = 0; i < population; i++) {
		delete [] score[i];
	}
	delete [] score;
	return;
}
void initial_population(int start, int population, int dimension, int ** chromosome, int * gene_upper){
	int random(int lower, int upper);
	for (int i = start; i < population; i++) {
		for (int j = 0; j < dimension; j++) {
			chromosome[i][j] = random(0,gene_upper[j]);
    		//cout << chromosome[i][j] << "\t";
		}
		//cout << endl;
	}
	return;
}
int random(int lower, int upper){
	return rand() % (upper - lower + 1) + lower;
}
void create_objective_matrix(int goods_num, double ** goods, int x_num, double * objective){
	//cout << "objective matrix:" << endl;
	for (int i = 0; i < x_num; i++) {
		objective[i] = goods[i % goods_num][3];
		//cout << objective[i] << "\t";
	}
	//cout << endl;
	return;
}
void create_constraints_matrix(	int goods_num, double ** goods, int space_num, double ** space,
								int x_num, int constraints_num, double ** constraints, double * RHS){
	for (int i = 0; i < space_num; i++) {    //各空間重量限制
		for (int j = 0; j < goods_num; j++) {
			constraints[i][i * goods_num + j] = goods[j][1];
		}
		RHS[i] = space[i][0];
	}
	for (int i = 0; i < space_num; i++) {    //各空間體積限制
		for (int j = 0; j < goods_num; j++) {
			constraints[i + space_num][i * goods_num + j] = goods[j][2];
		}
		RHS[i + space_num] = space[i][1];
	}
	double total_weight = 0;    //空間總重量容量
	for (int i = 0; i < space_num; i++) {
		total_weight += space[i][0];
	}
	for (int i = 0; i < space_num; i++) {    //各空間重量平衡限制
		for (int j = 0; j < x_num; j++) {
			constraints[i + 2 * space_num][j] = -goods[j % goods_num][1] * space[i][0] / total_weight;
		}
		for (int j = 0; j < goods_num; j++) {
			constraints[i + 2 * space_num][i * goods_num + j] += goods[j][1];
		}
		RHS[i + 2 * space_num] = 0;
	}
	for (int i = 0; i < goods_num; i++) {    //貨物數量限制
		for (int j = 0; j < space_num; j++) {
			constraints[i + 3 * space_num][j * goods_num + i] = 1;
		}
		RHS[i + 3 * space_num] = goods[i][0];
	}
	double * objective = new double[x_num]();    //目標函式係數
	for (int i = 0; i < x_num; i++) {            //coefficients of objective function
		objective[i] = goods[i % goods_num][3];
	}
	/*cout << "constraints matrix:" << endl;
	for (int i = 0; i < constraints_num; i++) {
		for (int j = 0; j < x_num; j++) {
			cout << constraints[i][j] << "\t";
		}
		if (i >= 2 * space_num && i < 3 * space_num) {
			cout << " = " << RHS[i] << endl;
		}else {
			cout << "<= " << RHS[i] << endl;
		}
	}*/
	return;
}
