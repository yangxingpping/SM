
import os;


#not support recursive
if __name__ == '__main__':
    for root, dirs, files in os.walk('./'):
        for file in files:
            curfile = os.path.join(root, file);
            nameArr = os.path.splitext(file);
            if curfile.endswith('.sql') == False:
                continue;
            os.system('python3 ./ddl2cpp {} {} {}'.format(curfile, nameArr[0], nameArr[0]));
