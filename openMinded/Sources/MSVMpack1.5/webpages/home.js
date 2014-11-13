var dimension = 0;

function chooseKernel() {
      var kernel = document.getElementById("kernel");
      var kernelpar = document.getElementById("kernelpar");   
   
      if(kernel.value == "1") {
      		kernelpar.disabled=true;
      }
      else {
      		kernelpar.disabled=false;
      }
      
      // set default kernelpar value
      switch (kernel.value) {
      	case "2":			// for Gaussian kernel
      		if(dimension > 0) 
      			kernelpar.value = Math.round(1000 * Math.sqrt(5 * dimension)) / 1000;
      		break;
      	
      	case "3":
      	case "4":
      		kernelpar.value = "3";	// for polynomial kernel
	      	break;
      	case "5":
      	case "6":
      	case "7":
      		kernelpar.value = "1.0";  // for custom kernels
      		break;
      	default:
	      	break;
      	
      }
}

function printInfo() {
	var model = document.getElementById("modeltest");
	if(model.value != "") {
		//window.open("home.cgi?modeltest=" + model.value + "#test");
		window.location.href="home.cgi?modeltest=" + model.value + "#test";
	}
}

function printDataInfo() {
	var data = document.getElementById("trainfile");
	if(data.value != "") {		
		window.location.href="home.cgi?trainfile=" + data.value; // + "#train";
	}
}

function UploadMSVMpack() {
	var formupload = document.getElementById("formupload");
	  formupload.action = "upload.cgi";
}

function UploadLibSVM() {
	var formupload = document.getElementById("formupload");
	  formupload.action = "upload_libsvm.cgi";
}
function UploadRaw() {
	var formupload = document.getElementById("formupload");
	  formupload.action = "upload_raw.cgi";
}

function setMultipleC() {
	var Clist = document.getElementsByName("Clist");
	var c= document.getElementsByName("C");
	var checkC = document.getElementsByName("multipleC");
	if(checkC.item(0).checked == false) {
		Clist.item(0).disabled = true;
		c.item(0).disabled = false;
	}
	else {
		Clist.item(0).disabled = false;
		c.item(0).disabled = true;
	}
}

