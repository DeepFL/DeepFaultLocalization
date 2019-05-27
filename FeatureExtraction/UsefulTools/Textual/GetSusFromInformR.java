import java.io.*;
import java.util.*;

public class GetSusFromInformR {

	public static void main(String[] args) throws IOException {
		ArrayList<String> MethodsCoveredbyF = new ArrayList<String>();
		
		String[] sixquerys={"MtoT","MtoTc","MtoTFS","McltoT","McltoTc","McltoTFS","MmtoT","MmtoTc",
				            "MmtoTFS","MvtoT","MvtoTc","MvtoTFS","McomtoT","McomtoTc","McomtoTFS"};
		String ID = args[0];
		String Project = args[1];
		String rootPath = args[2];
		BufferedReader MF = new BufferedReader(new FileReader(rootPath + "FinalFeatures/tempResult/"+Project+"/"+ID+"/Ample.txt"));
		String methodsName = MF.readLine();
		while(methodsName != null){
			MethodsCoveredbyF.add(methodsName.split(" ")[0]);
			methodsName = MF.readLine();
		}
		MF.close();
		
		
		
		String BasePath = rootPath + "RawFeatures/Textual/";
		for(int i=0;i<sixquerys.length;i++){
			BufferedReader MethodIDbr = new BufferedReader(new FileReader(
											BasePath + Project + "/" + ID + "/" + sixquerys[i] + "/Documents/"+"MethodID.txt"));
			HashMap<String,String> methodID = new HashMap<String,String>();
			StringBuilder alldata = new StringBuilder();
			String methodid = MethodIDbr.readLine();
			while(methodid!=null){
				alldata.append(methodid);
				methodid=MethodIDbr.readLine();
			}
			MethodIDbr.close();

			String[] adata=alldata.toString().split("M-");

			for(String d:adata){
				if(d.contains(":")){	
					String mID="M-"+d.split(":")[0];
					String mName=d.split(":")[1].replaceAll("\\s+", "");
					methodID.put(mID,mName);
				}
			}

			BufferedReader Resultbr=new BufferedReader(new FileReader(BasePath+Project+"/"+ID+"/"+sixquerys[i]+"/"+"Result.txt"));
			String Resultline=Resultbr.readLine();
			HashMap<String,ArrayList<Double>> mResults=new HashMap<String,ArrayList<Double>>();
			while(Resultline!=null){
				if(Resultline.contains("runName")){
					String[] items=Resultline.split(" ");
					String mID=items[2];
					String mName=methodID.get(mID);
					Double Score=Double.parseDouble(items[4].trim());
					if(!mResults.containsKey(mName)){
						ArrayList<Double> scores=new ArrayList<Double>();
						scores.add(Score);
						if(MethodsCoveredbyF.contains(mName))
							mResults.put(mName,scores);
					}else{
						if(MethodsCoveredbyF.contains(mName))
							mResults.get(mName).add(Score);
					}
				}
				Resultline=Resultbr.readLine();
			}
			Resultbr.close();
			
			BufferedWriter bw = new BufferedWriter(new FileWriter(rootPath + "FinalFeatures/Textual/" + Project + "/" + ID + "/tempResults/"+sixquerys[i]+".txt",true));
			for(String key:mResults.keySet()){
				Double MaxScore=Collections.max(mResults.get(key));
			
				bw.write(key+" "+MaxScore.toString());
				bw.write("\n");
			}
			bw.flush();
			bw.close();
		}
	}

}
