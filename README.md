# Projeto de ENG4033: Sistema de Leitura de Carteirinha para Concessão de Presenças

## Descrição do Projeto

Este projeto consiste na criação de um sistema de leitura de carteirinhas para a concessão de presenças em aulas de professores da PUC-Rio. Utilizando um microcontrolador, um leitor RFID, um cartão SD e um display TFT, o sistema é capaz de ler a carteirinha do aluno, verificar se ele está registrado e, se estiver, marcar a presença dele na aula. Caso o aluno não esteja registrado, o sistema oferece a opção de cadastrá-lo mediante a autenticação de uma senha mestra.

## Funcionalidades

- **Leitura de Carteirinhas RFID**: Utiliza um leitor RFID para ler o UID das carteirinhas dos alunos.
- **Verificação de Presença**: Verifica se o aluno está registrado e concede presença se o UID for encontrado no sistema.
- **Cadastro de Novos Alunos**: Permite o cadastro de novos alunos no sistema, mediante autenticação com uma senha mestra.
- **Armazenamento de Dados**: Utiliza um cartão SD para armazenar e gerenciar os dados dos alunos em formato JSON.
- **Interface Gráfica**: Exibe mensagens e instruções no display TFT.
