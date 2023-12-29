FROM docker.io/gcc:13.2.0-bookworm as build
RUN apt-get update && apt-get install -y libbluetooth-dev
COPY . /usr/src/govee-prometheus
WORKDIR /usr/src/govee-prometheus
RUN make -B clean bin/monitor

FROM scratch
WORKDIR /app
COPY --from=build /usr/src/govee-prometheus/bin/monitor ./
ENTRYPOINT ["./monitor"]
