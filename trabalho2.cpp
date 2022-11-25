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

//std::list<int> petOrder; // cat = 0 | dog = 1
//int last = 0;
bool bCat = true;
bool bDog = false;
int countEating = 0;
int countCats = 0;
int countDogs = 0;

semaphore* semaphoreInst = new semaphore(MAX_PETS);
std::list<std::string> texts;



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
		
//		while(bCat || (countEating != 0 && !bCat));	
		while(bCat || countDogs > 0);
		semaphoreInst->acquire();
		
		countEating++;	
		countCats++;
		
		std::string text = "pet ";
		text += std::to_string(id);
		text += "  ";
		text += "cat";
		text += "  ";
		text += "  entrou";
		text += "  countCats ";
		text += std::to_string(countCats);
		text += "  countDogs ";
		text += std::to_string(countDogs);
		texts.push_back(text);
		
		do_stuff(id, "cat", "eating");
		countEating--;
		countCats--;
		
		std::string text2 = "pet ";
		text2 += std::to_string(id);
		text2 += "  ";
		text2 += "cat";
		text2 += "  ";
		text2 += "  saiu";
		text2 += "  countCats ";
		text2 += std::to_string(countCats);
		text2 += "  countDogs ";
		text2 += std::to_string(countDogs);
		texts.push_back(text2);
		
		semaphoreInst->release();
	}
}

void dog(int const id)
{
//	semaphore semaphoreInst = semaphoreSingleton.getInst();

	while(true)
	{
		do_stuff(id, "dog", "playing");
//		petOrder.push_back(1);
		
		if (!bCat && bDog)
		{
			bCat = true;
			bDog = false;
		}
		
		
//		while(bDog || (countEating != 0 && !bDog));
		while(bDog || countCats > 0);
		semaphoreInst->acquire();
		
		countEating++;	
		countDogs++;
		
		std::string text = "pet ";
		text += std::to_string(id);
		text += "  ";
		text += "dog";
		text += "  ";
		text += "  entrou";
		text += "  countCats ";
		text += std::to_string(countCats);
		text += "  countDogs ";
		text += std::to_string(countDogs);
		texts.push_back(text);
		
//		last = 1;
		do_stuff(id, "dog", "eating");
		countEating--;
		countDogs--;
		
		std::string text2 = "pet ";
		text2 += std::to_string(id);
		text2 += "  ";
		text2 += "dog";
		text2 += "  ";
		text2 += "  saiu";
		text2 += "  countCats ";
		text2 += std::to_string(countCats);
		text2 += "  countDogs ";
		text2 += std::to_string(countDogs);
		texts.push_back(text2);
		
		semaphoreInst->release();
	}
}


