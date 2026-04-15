package com.krd.control_plane.service;

import com.krd.control_plane.entities.MarketData;
import com.krd.control_plane.repository.MarketDataRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;
import jakarta.annotation.PostConstruct;

@Service
public class MarketDataSubscriberService {

    // 1. O lugar correto do Autowired é AQUI (no nível da classe, fora dos métodos)
    @Autowired
    private MarketDataRepository repository;

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
                    String message = subscriber.recvStr(0);

                    if (message != null) {
                        String[] parts = message.split("\\|");

                        if (parts.length == 5) {
                            MarketData data = new MarketData();
                            data.setExchange(parts[0]);
                            data.setSymbol(parts[1]);
                            data.setBidPrice(Double.parseDouble(parts[2]));
                            data.setAskPrice(Double.parseDouble(parts[3]));
                            data.setImbalance(Double.parseDouble(parts[4]));

                            repository.save(data); // SALVA NO BANCO!
                            System.out.println("[GRAVADO NO DB] " + data.getSymbol() + " | Imbalance: " + data.getImbalance());
                        }
                    }
                } // 2. A chave do while que estava faltando precisa fechar aqui!

            } catch (Exception e) {
                System.err.println("[ERRO ZMQ] " + e.getMessage());
            }
        }).start();
    }
}