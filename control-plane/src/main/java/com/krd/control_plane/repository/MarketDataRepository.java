package com.krd.control_plane.repository;

import com.krd.control_plane.entities.MarketData;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface MarketDataRepository extends JpaRepository<MarketData, Long> {
}