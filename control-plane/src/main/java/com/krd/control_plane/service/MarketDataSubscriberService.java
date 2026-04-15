package com.krd.control_plane.service;
import org.springframework.stereotype.Service;
import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;
import jakarta.annotation.PostConstruct;

@Service
public class MarketDataSubscriberService {

    @PostConstruct
    public void startListening() {
        // Criamos uma Thread separada (background) para o loop infinito do ZeroMQ
        // não travar a inicialização do Spring Boot.
        new Thread(() -> {
            try (ZContext context = new ZContext()) {
                System.out.println("=========================================");
                System.out.println("[TXSoft Control Plane] Inicializando ZMQ");
                System.out.println("=========================================");

                // Cria o socket tipo SUBSCRIBER (Receptor)
                ZMQ.Socket subscriber = context.createSocket(SocketType.SUB);

                // Conecta na porta onde o C++ está gritando
                subscriber.connect("tcp://localhost:5555");

                // Inscreve-se para receber TODAS as mensagens (filtro vazio)
                subscriber.subscribe("".getBytes());

                System.out.println("[JAVA-ZMQ] Conectado na porta 5555! Aguardando Motor C++...\n");

                // Loop infinito para ficar escutando
                while (!Thread.currentThread().isInterrupted()) {
                    // Fica bloqueado aqui até o C++ mandar algo
                    String message = subscriber.recvStr(0);

                    if (message != null) {
                        System.out.println("[SPRING BOOT CAPTUROU] " + message);
                    }
                }
            } catch (Exception e) {
                System.err.println("[ERRO ZMQ] " + e.getMessage());
            }
        }).start();
    }
}