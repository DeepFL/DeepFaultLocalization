
INDRIJAR=../../../swig/src/java/indri.jar
CLASSPATH=-classpath $(INDRIJAR)

all: jar

clean:
	rm src/edu/umass/cs/indri/*/*.class
	rm -f indrilib.jar

hits:
	javac $(CLASSPATH) src/edu/umass/cs/indri/hits/*.java

web:
	javac $(CLASSPATH) src/edu/umass/cs/indri/web/*.java

jar: web hits
	rm -f indrilib.jar
	cd src; jar cvf ../indrilib.jar edu 

