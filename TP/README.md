# Rastreamento e Monitorização da Execução de Programas

### Concept
> "Pretende-se implementar um serviço de monitorização dos programas executados numa máquina.\
> Os utilizadores devem conseguir executar programas, através do cliente, e obter o seu tempo de execução (i.e., o tempo total desde que o utilizador pede ao cliente para correr o programa até ao término do mesmo).\
> Um administrador de sistemas deve conseguir consultar, através do servidor, todos os programas que se encontram atualmente em execução, incluíndo o tempo dispendido pelos mesmos. Finalmente, o servidor deve também permitir a consulta de estatísticas sobre programas já terminados (p.ex., o tempo de execução agregado de um certo conjunto de programas)."

### Requirements

> C
> Linux based OS

### Running the project

> **_NOTES:_**
> - Run Monitor and each instance of Tracer in a separate Terminal with privileges
> - Several makefile targets are available

Navigate to the directory and run:
```bash
make
```
Followed by:
```bash
python3 monitor.py
```
```bash
python3 tracer.py
```
