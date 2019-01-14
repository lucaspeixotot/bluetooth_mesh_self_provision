#!/bin/bash
nrfjprog --eraseall -f nrf52
nrfjprog --program zephyr/zephyr.hex -f nrf52
nrfjprog --reset -f nrf52
