Ray lidar;
ArrayList<Reflector> reflectors;
ClusterEngine brain;
ArrayList<Vertex> rays;

int N = 500;
int noise = 10;

void setup(){
  size(1200,800);
  frameRate(60);
  
  randomSeed(58);

  brain = new ClusterEngine(5);
  reflectors = new ArrayList<Reflector>();
  rays = new ArrayList<Vertex>();
  
  lidar = new Ray(width/2, height/2,500);
  reflectors.add(new Reflector(random(width), random(height), 50));
  

}

void draw(){
  rays.clear();
  background(52);
  
  lidar.update(300);
  brain.compute(5);
  
  for(int i = 0; i<rays.size();i++) rays.get(i).draw();

  brain.draw();
  
}



void mousePressed(){
  println("[Event] : Mouse pressed !");
  
  if(mousePressed && mouseButton == LEFT ){
    reflectors.add(new Reflector(mouseX, mouseY, 50));
  }else if(mousePressed && mouseButton == RIGHT && reflectors.size() > 0){
    
    NearScan scan = new NearScan();
    scan.start(new PVector(mouseX, mouseY));
    for(int k = 0; k < reflectors.size(); k++)scan.run(reflectors.get(k).pos, k); 
    
    reflectors.remove(scan.result());
    
  }
  

  print("[Info] : "); 
  print(reflectors.size()); 
  println(" in scene with ", brain.data.size(), " points");
  
  background(52);
  
}


void sort(){
  ArrayList<Reflector> unsortedReflectors = reflectors ;
  ArrayList<Reflector> sortedReflectors = new ArrayList<Reflector>() ;
  
  while(unsortedReflectors.size() > 0){
    float min = 99999;
    Reflector buffer = new Reflector(0,0,0);
   
    for(int i = 0; i < unsortedReflectors.size(); i++){
      float dist = PVector.dist(unsortedReflectors.get(i).pos, lidar.origin);
      if(dist < min){
        min = dist;
        buffer = unsortedReflectors.get(i);
      }
    }
    sortedReflectors.add(buffer);
    unsortedReflectors.remove(buffer);
  }
  
  reflectors = sortedReflectors;
  
}
