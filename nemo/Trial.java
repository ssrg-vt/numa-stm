import java.util.concurrent.atomic.AtomicInteger;

public class Trial extends Thread {
	static AtomicInteger owner = new AtomicInteger(1), owner_in = new AtomicInteger(0), request = new AtomicInteger(0);
	static volatile int counter = 0;
	static volatile boolean[] in = new boolean[40];
	static volatile int[] flags = new int[40];
	int id;
	public Trial(int id) {
		this.id = id;
	}
	public void run() {
		for (int kk = 0; kk < 1000000; kk++) {
			int granted = 0;
			
			while (granted == 0) {
				flags[id] = 1;
				if (request.get() == 0 && owner.get() == id) {
					
					
					owner_in.set(id);
					
					if (owner.get() == id) {
						
						granted = 1;
						
					}
					else {
						flags[id] = 0;
						owner_in.set(-id);
//						owner_in.compareAndSet(id, 0);
						
					}
				} else if (request.compareAndSet(0, id)) {
					int old_owner = owner.get();
					owner.set(id);
					
					while (owner_in.get() != 0 && owner_in.get() == old_owner) {
						yield();
					}
					if (owner_in.get() != 0) {
						flags[id] = 0;
						while (flags[old_owner] != 0){
							yield();
						}
						owner_in.set(0);
						request.set(0);
					} else {
						granted = 2;
					}
				} else {
					flags[id] = 0;
				}
				
			}
			
			in[id] = true;
			
			counter++;
			
			while (in[1] && in[2]);
			in[id] = false;
			
			if (granted == 2) {
				
				request.set(0);
				
			} else {
				
				owner_in.set(0);
				
			}
			flags[id] = 0;
		}
	}

	
	public static void main(String[] args) {
		int threadsCount = 4;
		
		Trial[] th = new Trial[40];
		
		for (int i=0; i < threadsCount; i++) {
			th[i] = new Trial(i+1);
			th[i].start();
		}
		
		try {
			for (int i=0; i < threadsCount; i++) {
				th[i].join();
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		System.out.println(counter);
	}
}
