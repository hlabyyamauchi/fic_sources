def weight_to_header(openfile, savefile, macro_name):
    with open(openfile) as f:
        data = f.read()
        data = data.replace('\n',',')
        head = '#define ' + str(macro_name) + ' {'
        foot = '}'
        content = head + data + foot
    with open(savefile, 'w') as f:
        f.write(content)
if __name__ == "__main__":
    weight_to_header('conv1_w.txt','conv1_w.h','CONV1_W')
    weight_to_header('conv1_b.txt','conv1_b.h','CONV1_B')
    weight_to_header('conv2_w.txt','conv2_w.h','CONV2_W')
    weight_to_header('conv2_b.txt','conv2_b.h','CONV2_B')
    weight_to_header('fc1_w.txt','fc1_w.h','FC1_W')
    weight_to_header('fc1_b.txt','fc1_b.h','FC1_B')
    weight_to_header('fc2_w.txt','fc2_w.h','FC2_W')
    weight_to_header('fc2_b.txt','fc2_b.h','FC2_B')
    weight_to_header('./image1000/image000.txt','image000.h','IMAGE000')