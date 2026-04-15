package com.krd.control_plane;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;

// Desliga a autoconfiguração do Banco de Dados temporariamente
@SpringBootApplication(exclude = {DataSourceAutoConfiguration.class})
public class ControlPlaneApplication {

	public static void main(String[] args) {
		SpringApplication.run(ControlPlaneApplication.class, args);
	}
}