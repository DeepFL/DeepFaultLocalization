package com.virtualmachinery.jhawk.demo.custommetric;

import com.virtualmachinery.jhawk.registry.basemetrics.BaseIntMetric;
import com.virtualmachinery.jhawk.registry.interfaces.JHawkMetricInterface;

public class MyCustomMetric extends BaseIntMetric implements
		JHawkMetricInterface {

	public int calcBasic(Object arg0) {
		return 999;
	}

	public String getXMLTag() {
		return "mine";
	}

	public String getHTMLFeatureRow(Object arg0) {
		return 	createHTMLWideFeatureRow("My Custom Metric ",this.calculate(arg0));
	}

}
