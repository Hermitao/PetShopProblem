/*
	CURITIBA 11/2022
	UNIVERSIDADE POSITIVO
	PROGRAMACAO CONCORRENTE/DESENVOLVIMENTO DE SISTEMAS
	
	TRABALHO 2
	- ADAPTACAO DO "PROBLEMA DO BANHEIRO UNICO" (VER DETALHES NA ESPECIFICACAO)
	
	TAREFA
	- COMPLETAR/COMPLEMENTAR AS FUNCOES "CAT" E "DOG" NO FINAL DESTE ARQUIVO
	
	REGRAS
	- VOCE PODE ADICIONAR CODIGO A VONTADE DESDE QUE SEJA A PARTIR DA LINHA COM O COMENTARIO "TODO"
	- VOCE PODE INCLUIR CABECALHOS A VONTADE
	- NADA DO QUE ESTA ESCRITO PODE SER APAGADO
	
	INFORMACOES IMPORTANTES
	- A ACAO "EATING" EH CRITICA, A ACAO "PLAYING" EH NAO-CRITICA
	- DEVE HAVER EXCLUSAO MUTUA ENTRE GATOS E CACHORROS NA AREA DE COMIDA
	- O NUMERO DE PETS NA AREA DE COMIDA NAO PODE ULTRAPASAR O VALOR DA MACRO "MAX_PETS"
	- NAO DEVE HAVER STARVATION DE GATOS OU CACHORROS
	
	DICAS
	- HA UMA CLASSE "SEMAFORO" DISPONIVEL PARA USO
	- LEMBRE-SE DE COMPILAR PARA C++11 (-std=c++11) OU SUPERIOR
	- A COMPREENSAO DO CODIGO EH PARTE DO TRABALHO
*/

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <list>
#include <iostream>


class semaphore
{
	long count;
	std::mutex mtx;
	std::condition_variable cv;
	
	public:
	
	semaphore(long const c = 0) : count(c) {}
	
	semaphore(semaphore const &) = delete;
	semaphore(semaphore &&) = default;
	semaphore & operator=(semaphore const &) = delete;
	semaphore & operator=(semaphore &&) = default;
	~semaphore() = default;
	
	void acquire() //aka "wait", "down", "p"
	{
		auto lock = std::unique_lock<std::mutex>(mtx);
		while(!count) cv.wait(lock);
		--count;
	}
	
	void release() //aka "signal", "up", "v"
	{
		auto lock = std::unique_lock<std::mutex>(mtx);
		++count;
		cv.notify_one();
	}
};

#define MAX_PETS 16
#define MAX_SLEEP_US 4
#define NUM_THREADS 100

void do_stuff(int const id, char const * kind, char const * action)
{
	std::printf("pet #%d (%s) started %s...\n", id, kind, action);
	std::this_thread::sleep_for(std::chrono::microseconds(std::rand() % MAX_SLEEP_US));
	std::printf("pet #%d (%s) stopped %s...\n", id, kind, action);
}

void cat(int const);
void dog(int const);

int main()
{
	auto pets = std::vector<std::thread>(NUM_THREADS);
	
	for(int i = 0; i < pets.size(); ++i)
	{
		pets.at(i) = std::thread(i % 2 ? cat : dog, i);
	}
	
	for(int i = 0; i < pets.size(); ++i)
	{
		pets.at(i).join();
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////
//TODO
//////////////////////////////////////////////////////////////

bool bCat = true;
bool bDog = false;
int countEating = 0;
int countCats = 0;
int countDogs = 0;

semaphore* semaphoreInst = new semaphore(MAX_PETS);

class Peterson {
	private:
	volatile int interested[2] = {0, 0};
	volatile int turn = 0;
	
	public:
	
	void lock(int tid) {
		interested[tid] = 1;
		
		int other = 1 - tid;
		turn = other;
		
		while (turn == other && interested[other])
			;
	}
	
	void unlock(int tid) {
		interested[tid] = 0;
	}
};

Peterson* peterson = new Peterson();

void cat(int const id)
{
	while(true)
	{
		do_stuff(id, "cat", "playing");

		if (!bDog && bCat)
		{
			bDog = true;
			bCat = false;
		}
		
		while(bCat || countDogs > 0);
		semaphoreInst->acquire();
		
		peterson->lock(id);
		
		countEating++;	
		countCats++;
		
		do_stuff(id, "cat", "eating");
		countEating--;
		countCats--;

		peterson->unlock(id);
		
		semaphoreInst->release();
		
		break;
	}
}

void dog(int const id)
{
	while(true)
	{
		do_stuff(id, "dog", "playing");

		if (!bCat && bDog)
		{
			bCat = true;
			bDog = false;
		}
		
		while(bDog || countCats > 0);
		semaphoreInst->acquire();
		
		peterson->lock(id);
		
		countEating++;	
		countDogs++;
		
		do_stuff(id, "dog", "eating");
		countEating--;
		countDogs--;
		
		peterson->unlock(id);
		
		semaphoreInst->release();
		
		break;
	}
}


