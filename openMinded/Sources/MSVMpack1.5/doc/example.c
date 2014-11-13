#include "libMSVM.h"        // Generic structure and function declarations
#include "libtrainMSVM.h"   // Training functions (not required for predictions only)
#include "libevalMSVM.h"    // Evaluation functions (also used during training)

int main(void) {
    struct Model *model;                    // declare a model
    struct Data *training_set, *test_set;   // declare the datasets
    
    long status;                // for MSVM_train return code
    double accuracy = 0.98;     // desired accuracy level of 98%  
    long chunk_size = 4;        // size of the chunk used for training
    int cache_memory = 0;       // Amount of cache memory (0 = max)
    int nprocs = 0;             // Number of available CPUs (0 = all)
    long *labels;
    
    printf("This is a simple example showing how to use\n MSVMpack through the C API.\n\n");
            
    model = MSVM_make_model(MSVM2); // Create an empty model with default parameters
    if(model == NULL) {
        printf("Error in model creation\n");
        exit(1);
    }
    else 
        printf("M-SVM model successfully created with defaults:\n MSVM2-type, kernel type = %d.\n", model->nature_kernel);

    // Change some parameters
    model->nature_kernel = RBF;    
    printf("Changed the kernel type in model to Gaussian RBF.\n");
    
    MSVM_model_set_C(1.0, 3, model);
    printf("Initialized the hyperparameters C to %1.2lf for all %ld classes.\n",model->C[1],model->Q);
    model->C[2] = 2.0;
    model->C[3] = 3.0;
    printf("Changed hyperparameters C_2 to %1.2lf and C_3 to %1.2lf.\n",model->C[2],model->C[3]);

    printf("Loading the data... \n");
    // The data format can be either DOUBLE, FLOAT, INT, SHORT, BYTE, or BIT
    training_set = MSVM_make_dataset("myTrainingData",DOUBLE); // load the training data
    test_set = MSVM_make_dataset("myTestData",DOUBLE);         // load the test data
        
    printf("Calling MSVM_train()...\n");
    /* Train the model on the training_set
        with default initialization (first NULL)
        no periodic saving of alpha (second NULL)
        no log file (third NULL)
    */
    status = MSVM_train(model, training_set, chunk_size, accuracy, 
                         cache_memory, nprocs, NULL, NULL, NULL);
        
    if(status >= 0) {
        printf("Training done without error, will now classify the test set.\n");

	/* Allocate the array for predicted labels 
		(size should be (size of test set + 1) )
	*/
        labels = (long *)malloc(sizeof(long) * (test_set->nb_data + 1));

        /* Classify the test set,
        	store the predicted labels in 'labels'
        	and print outputs on screen (because filename=NULL)
        */ 
        MSVM_classify_set(labels, test_set->X, test_set->y, test_set->nb_data, 
                           NULL, model, nprocs);
    }
    else
        printf("Error in training.\n");
        
    // Free the memory
    free(labels);
    MSVM_delete_model(model);    
    MSVM_delete_dataset(training_set);
    MSVM_delete_dataset(test_set);

    return 0;
}

