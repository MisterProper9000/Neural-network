#include <iostream>
#include <stdlib.h>
#include <tchar.h>
#include <cstdio>
#include <cstdarg>

#include <vld.h>
#include <string>
#include <time.h>

#define	  stop __asm nop	

using namespace std;



#define INPUT_NEURONS		4
#define HIDDEN_NEURONS		3
#define OUTPUT_NEURONS		4

#define LEARN_RATE			0.15		//коэффициент обучения
#define RAND_WEIGHT			( ((float)rand() / (float)RAND_MAX) - 0.5 )
#define getSRand()			( (float)rand() / (float)RAND_MAX )

#define getRand(x)			(int)((x) * getSRand())
#define sqr(x)				((x) * (x))

/*    ВЕСА    */
//вход скрытых ячеек (со смещением)
double wih[INPUT_NEURONS + 1][HIDDEN_NEURONS];

//вход выходных ячеек (со смещением)
double who[HIDDEN_NEURONS + 1][OUTPUT_NEURONS];

//активаторы
double inputs[INPUT_NEURONS];
double hidden[HIDDEN_NEURONS];
double target[OUTPUT_NEURONS];
double actual[OUTPUT_NEURONS];

//ошибки
double erro[OUTPUT_NEURONS];
double errh[HIDDEN_NEURONS];

void assignRandWeights(void);
double sigmoid(double val);
double sigmoidDerivative(double val);
void feedForward();
void backPropagate(void);

typedef struct {
	double health;
	double knife;
	double gun;
	double enemy;
	double out[OUTPUT_NEURONS];
} ELEMENT;

#define MAX_SAMPLES 18
//первый элемент -- интеллект, второй -- число заваленных кр (отношения с преподом), третий -- отупляющий дебафф, четвёртый -- дни на подготовку
/*	   I	F    S    D     N    R	  Z    P */
ELEMENT samples[MAX_SAMPLES] = {
	{ 2.0, 0.0, 0.0, 1.0,{ 0.0, 1.0, 0.0, 0.0 } },
	{ 2.0, 0.0, 0.0, 2.0,{ 1.0, 0.0, 0.0, 0.0 } },
	{ 2.0, 0.0, 1.0, 1.0,{ 0.0, 1.0, 0.0, 0.0 } },
	{ 2.0, 0.0, 1.0, 2.0,{ 1.0, 0.0, 0.0, 0.0 } },
	{ 2.0, 1.0, 0.0, 2.0,{ 0.0, 1.0, 0.0, 0.0 } },
	{ 2.0, 1.0, 0.0, 4.0,{ 1.0, 0.0, 0.0, 0.0 } },
	{ 1.0, 0.0, 0.0, 0.0,{ 0.0, 0.0, 0.0, 1.0 } },
	{ 1.0, 0.0, 0.0, 1.0,{ 0.0, 0.0, 1.0, 0.0 } },
	{ 1.0, 0.0, 1.0, 1.0,{ 0.0, 0.0, 0.0, 1.0 } },
	{ 1.0, 0.0, 1.0, 2.0,{ 0.0, 0.0, 1.0, 0.0 } },
	{ 1.0, 1.0, 0.0, 3.0,{ 0.0, 1.0, 0.0, 0.0 } },
	{ 1.0, 0.0, 0.0, 5.0,{ 1.0, 0.0, 0.0, 0.0 } },
	{ 0.0, 0.0, 0.0, 0.0,{ 0.0, 0.0, 0.0, 1.0 } },
	{ 0.0, 0.0, 0.0, 1.0,{ 0.0, 0.0, 0.0, 1.0 } },
	{ 0.0, 0.0, 1.0, 1.0,{ 0.0, 0.0, 0.0, 1.0 } },
	{ 0.0, 0.0, 1.0, 2.0,{ 0.0, 0.0, 1.0, 0.0 } },
	{ 0.0, 1.0, 0.0, 2.0,{ 0.0, 0.0, 1.0, 0.0 } },
	{ 0.0, 1.0, 0.0, 1.0,{ 0.0, 0.0, 0.0, 1.0 } }
};

char *strings[4] = { "Keep calm", "prepare, teach!", "ass-tearing", "Tebe kranti" };

int action(double *vector)
{
	int index, sel;
	double max;
	sel = 0;
	max = vector[sel];
	for (index = 1; index < OUTPUT_NEURONS; index++) {
		if (vector[index] > max) {
			max = vector[index]; sel = index;
		}
	}
	return(sel);
}



void assignRandWeights(void) {
	int inp, hid, out;

	for (inp = 0; inp < INPUT_NEURONS + 1; inp++) {
		for (hid = 0; hid < HIDDEN_NEURONS; hid++) {
			wih[inp][hid] = RAND_WEIGHT;
		}
	}

	for (hid = 0; hid < HIDDEN_NEURONS + 1; hid++) {
		for (out = 0; out < OUTPUT_NEURONS; out++) {
			who[hid][out] = RAND_WEIGHT;
		}
	}
}

double sigmoid(double val) {
	return (1.0 / (1.0 + exp(-val)));
}

double sigmoidDerivative(double val) {
	return (val * (1.0 - val));
}

void feedForward() {
	int inp, hid, out;
	double sum;

	//вычисляем входы в скрытые слои
	for (hid = 0; hid < HIDDEN_NEURONS; hid++) {

		sum = 0.0;
		for (inp = 0; inp < INPUT_NEURONS; inp++) {
			sum += inputs[inp] * wih[inp][hid];
		}


		//добавить смещение
		sum += wih[INPUT_NEURONS][hid];

		hidden[hid] = sigmoid(sum);
	}

	//вход во входной слой
	for (out = 0; out < OUTPUT_NEURONS; out++) {

		sum = 0.0;
		for (hid = 0; hid < HIDDEN_NEURONS; hid++) {
			sum += hidden[hid] * who[hid][out];
		}

		//äîáàâèòü ñìåùåíèå
		sum += who[HIDDEN_NEURONS][out];

		actual[out] = sigmoid(sum);
	}

}

void backPropagate(void) {
	int inp, hid, out;

	//вычисление ошибки выходного слоя
	for (out = 0; out < OUTPUT_NEURONS; out++) {
		erro[out] = (target[out] - actual[out]) * sigmoidDerivative(actual[out]);
	}

	//вычисление ошибки скрытого слоя
	for (hid = 0; hid < HIDDEN_NEURONS; hid++) {
		errh[hid] = 0.0;
		for (out = 0; out < OUTPUT_NEURONS; out++) {
			errh[hid] += erro[out] * who[hid][out];
		}

		errh[hid] *= sigmoidDerivative(hidden[hid]);
	}

	//обновление весов выходного слоя
	for (out = 0; out < OUTPUT_NEURONS; out++) {
		for (hid = 0; hid < HIDDEN_NEURONS; hid++) {
			who[hid][out] += (LEARN_RATE * erro[out] * hidden[hid]);
		}

		//обновление смещения
		who[HIDDEN_NEURONS][out] += (LEARN_RATE * erro[out]);
	}

	//обновление весов скрытого слоя
	for (hid = 0; hid < HIDDEN_NEURONS; hid++) {
		for (inp = 0; inp < INPUT_NEURONS; inp++) {
			wih[inp][hid] += (LEARN_RATE * errh[hid] * inputs[inp]);
		}

		//обновление смещения
		wih[INPUT_NEURONS][hid] += (LEARN_RATE * errh[hid]);
	}
}



int main()
{
	{
		double err;
		int i, sample = 0, iterations = 0;
		int sum = 0;
		FILE * out = fopen("stats.txt", "w");
		/* Инициализировать генератор случайных чисел */
		srand(time(NULL));
		assignRandWeights();
		/* Обучить сеть */
		while (1) {
			if (++sample == MAX_SAMPLES) sample = 0;

			inputs[0] = samples[sample].health;
			inputs[1] = samples[sample].knife;
			inputs[2] = samples[sample].gun;
			inputs[3] = samples[sample].enemy;

			target[0] = samples[sample].out[0];
			target[1] = samples[sample].out[1];
			target[2] = samples[sample].out[2];
			target[3] = samples[sample].out[3];

			feedForward();

			err = 0.0;
			for (i = 0; i < OUTPUT_NEURONS; i++) {
				
				err += sqr((samples[sample].out[i] - actual[i]));
			}
			err = 0.5 * err;

			fprintf(out, "%g\n", err);
			printf("mse = %g\n", err);
			if (iterations++ > 100000) break;
			backPropagate();
		}
		/* Проверить сеть */
		for (i = 0; i < MAX_SAMPLES; i++) {
			inputs[0] = samples[i].health;
			inputs[1] = samples[i].knife;
			inputs[2] = samples[i].gun;
			inputs[3] = samples[i].enemy;
			target[0] = samples[i].out[0];
			target[1] = samples[i].out[1];
			target[2] = samples[i].out[2];
			target[3] = samples[i].out[3];
			feedForward();
			if (action(actual) != action(target)) {
				fprintf(out, "%2.1g:%2.1g:%2.1g:%2.1g %s (%s)\n",
					inputs[0], inputs[1], inputs[2], inputs[3],
					strings[action(actual)], strings[action(target)]);
			}
			else {
				sum++;
			}
		}
		fprintf(out, "Network is %g%% correct\n",
			((float)sum / (float)MAX_SAMPLES) * 100.0);
		/* Выполнение тестов */
		/* Интеллект Кр Дебафф Дни */

		inputs[0] = 2.0; inputs[1] = 2.0; inputs[2] = 2.0; inputs[3] = 1.0;
		feedForward();

		fprintf(out, "2221 Action %s\n", strings[action(actual)]);
		inputs[0] = 1.0; inputs[1] = 0.0; inputs[2] = 1.0; inputs[3] = 1.0;
		feedForward();

		fprintf(out, "1011 Action %s\n", strings[action(actual)]);
		inputs[0] = 0.0; inputs[1] = 0.0; inputs[2] = 0.0; inputs[3] = 0.0;
		feedForward();

		fprintf(out, "0000 Action %s\n", strings[action(actual)]);
		inputs[0] = 0.0; inputs[1] = 1.0; inputs[2] = 0.0; inputs[3] = 3.0;
		feedForward();

		fprintf(out, "0103 Action %s\n", strings[action(actual)]);
		inputs[0] = 2.0; inputs[1] = 0.0; inputs[2] = 1.0; inputs[3] = 3.0;
		feedForward();

		fprintf(out, "1013 Action %s\n", strings[action(actual)]);
		inputs[0] = 2.0; inputs[1] = 1.0; inputs[2] = 0.0; inputs[3] = 1.0;
		feedForward();

		fprintf(out, "2101 Action %s\n", strings[action(actual)]);
		inputs[0] = 0.7; inputs[1] = 1.0; inputs[2] = 0.7; inputs[3] = 3.5;
		feedForward();
		
		fprintf(out, "0.7 1 0.7 3.5 Action %s\n", strings[action(actual)]);
		fclose(out);
		return 0;
	}
	stop
}
