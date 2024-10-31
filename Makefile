all:
	cd ESLSCDLL && make && cd .. && \
	cd CsimpleExample && make && cd .. && \
	cd escam && qmake && make && cd .. && \
	cd escam_deb && ./create_escam_deb.sh

clean:
	cd ESLSCDLL && make clean && cd .. && \
	cd CsimpleExample && make clean && cd .. && \
	cd escam && qmake && make clean