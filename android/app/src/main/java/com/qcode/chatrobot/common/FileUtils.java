package com.qcode.chatrobot.common;

import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.security.MessageDigest;

public abstract class FileUtils {
	private FileUtils() {
	}

	public static String basename(String path) {
		return new File(path).getName();
	}

	public static String dirname(String path) {
		return new File(path).getParent();
	}

	public static boolean mkdir(String dirpath) {
		File dir = new File(dirpath);
		if (dir.isDirectory() == true) {
			return true;
		}

		boolean ret = dir.exists();
		if (ret == false) {
			ret = dir.mkdirs();
		}

		if (ret == false) {
			Log.w(TAG, "FileUtils.mkdir() failed, path=" + dirpath);
		}
		return ret;
	}

	public static boolean mkdir(String dirpath, int mode) {
		boolean ret = mkdir(dirpath);
		if (ret == true) {
			ret = chmod(dirpath, mode);
		}
		return ret;
	}

	public static boolean chmod(String path, int mode) {
		int ret = -1;
		try {
			String command = "chmod -R 0" + mode + " " + path;
			Process process = Runtime.getRuntime().exec(command);
			ret = process.waitFor();
		} catch (Exception e) {
			Log.e(TAG, "FileUtils.chmod() failed", e);
		}

		if (ret != 0) {
			Log.w(TAG, "FileUtils.chmod() failed, path=" + path + " mode=" + mode);
		}

		return (ret == 0);
	}

	static public boolean ln(String target, String link_name) {
		int ret = -1;
		String cmdline = "ln -s " + target + " " + link_name;
		try {
			Process process = Runtime.getRuntime().exec(cmdline);
			ret = process.waitFor();
		} catch (Exception e) {
			Log.e(TAG, "FileUtils.ln() failed", e);
		}

		if (ret != 0) {
			Log.w(TAG, "FileUtils.ln() failed, target=" + target + " link_name=" + link_name);
		}

		return (ret == 0);
	}

	public static boolean mv(String src, String dest) {
		File from = new File(src);
		File to = new File(dest);

		if (from.exists() == false) {
			Log.w(TAG, "FileUtils.mv() failed to move " + src + " ==> " + dest + ". src not exist.");
		}
		rm(dest);

		boolean ret = from.renameTo(to);
		if (ret == false) {
			Log.w(TAG, "FileUtils.mv() failed to move " + src + " ==> " + dest);
		}

		return ret;
	}

	public static boolean cp(String src, String dest) {
		long timestamp = System.currentTimeMillis();
		int ret = -1;
		try {

			String command = "cp -r " + src + " " + dest;
			Process process = Runtime.getRuntime().exec(command);
			ret = process.waitFor();
		} catch (Exception e) {
			Log.e(TAG, "FileUtils.cp() failed", e);
		}

		if (ret != 0) {
			Log.w(TAG, "FileUtils.cp() failed, from=" + src + " to=" + dest);
		}

		timestamp = System.currentTimeMillis() - timestamp;
		Log.d(TAG, "FileUtils.cp() copy '" + src + "' cost " + timestamp + "ms");
		return (ret == 0);
	}

	public static boolean rm(String path) {
		if (path == null) {
			Log.i(TAG, "AppUtils.rm(): parameter is null");
			return false;
		}

		boolean ret = false;
		File dir_or_file = new File(path);
		try {
			if (isSymlink(dir_or_file) == false
					&& dir_or_file.isDirectory()) {
				String[] children = dir_or_file.list();
				for (int i = 0; i < children.length; i++) {
					ret = rm(path + "/" + children[i]);
					if (ret == false) {
						break;
					}
				}
			}
			dir_or_file.delete();
			ret = true;
		} catch (Exception e) {
			Log.e(TAG, "FileUtils.rm() failed", e);
		}

		if (ret == false) {
			Log.w(TAG, "FileUtils.rm() failed, path=" + path);
		}
		return ret;
	}

	public static boolean isSymlink(File file) {
		try {
			File canon;
			if (file.getParent() == null) {
				canon = file;
			} else {
				File canonDir;
				canonDir = file.getParentFile().getCanonicalFile();
				canon = new File(canonDir, file.getName());
			}
			return !canon.getCanonicalFile().equals(canon.getAbsoluteFile());
		} catch (IOException e) {
			Log.e(TAG, "FileUtils.isSymlink() failed", e);
			return false;
		}
	}

	public static boolean canRead(String path) {
		if (path == null) {
			Log.i(TAG, "FileUtils.canRead(): parameter is null");
			return false;
		}

		File file = new File(path);
		boolean can_read = file.canRead();
		return can_read;
	}

	public static boolean exists(String path) {
		if (path == null) {
			Log.i(TAG, "FileUtils.exists(): parameter is null");
			return false;
		}

		File file = new File(path);
		boolean exists = file.exists();
		return exists;

	}

	public static String getMD5Sum(String path) {
		//Using MessageDigest update() method to provide input
		String md5sum = null;

		byte[] buffer = new byte[8192];
		int num;

		MessageDigest md = null;
		try {
			md = MessageDigest.getInstance("MD5");
			FileInputStream fis = new FileInputStream(path);
			while ((num = fis.read(buffer)) > 0) {
				md.update(buffer, 0, num);
			}
			fis.close();
		} catch (Exception e) {
			Log.e(TAG, "FileUtils.getMD5Sum() failed", e);
			return null;
		}

		byte[] hash = md.digest();

		// convert hash byte to string
		//md5sum = new BigInteger(1, hash).setBit(128).toString(16); // leading 0 is not correct
		final char[] HEX_DIGITS = "0123456789abcdef".toCharArray();
		char[] chars = new char[hash.length * 2];
		for (int i = 0; i < hash.length; i++) {
			chars[i * 2] = HEX_DIGITS[(hash[i] >> 4) & 0xf];
			chars[i * 2 + 1] = HEX_DIGITS[hash[i] & 0xf];
		}
		md5sum = new String(chars);

		return md5sum;
	}

	public static boolean checkMD5Sum(String path, String md5val) {
		String md5sum = getMD5Sum(path);
		if (md5sum == null) {
			return false;
		}

		int ret = md5sum.compareToIgnoreCase(md5val);
		return (ret == 0);
	}

	public static String readToString(String file_name) {
		InputStreamReader input_reader = null;
		BufferedReader buf_reader = null;
		File f = new File(file_name);

		try {
			input_reader = new InputStreamReader(new FileInputStream(f), "UTF-8");
			buf_reader = new BufferedReader(input_reader);
			String line;
			boolean first_line = true;
			String result = "";
			while ((line = buf_reader.readLine()) != null) {
				if (!first_line)
					result += "\n";
				else
					first_line = false;
				result += line;
			}

			return result;
		} catch (Exception e) {
			Log.e(TAG, "Failed to read file: " + file_name, e);
			return null;
		} finally {
			try {
				if (buf_reader != null) buf_reader.close();
				if (input_reader != null) input_reader.close();
			} catch (IOException e) {
				Log.e(TAG, "Unable to close file reader.", e);
			}
		}
	}

	public static boolean writeToFile(String file_name, String content) {
		if (file_name == null || content == null) {
			return false;
		}

		OutputStreamWriter output_writer = null;
		BufferedWriter buf_writer = null;

		try {
			output_writer = new OutputStreamWriter(new FileOutputStream(file_name), "UTF-8");
			buf_writer = new BufferedWriter(output_writer);

			buf_writer.write(content);

			return true;
		} catch (IOException ex) {
		} finally {
			try {
				if (buf_writer != null) buf_writer.close();
				if (output_writer != null) output_writer.close();
			} catch (Exception e) {
				Log.e(TAG, "Unable to close file writer.", e);
			}
		}

		return false;
	}

	public static void writeToFile(InputStream is, File dest, boolean replace) {
		if (!replace && dest.exists()) {
			return;
		}

		try {
			Log.d(TAG, "FileUtils.writeToFile() target=" + dest.getAbsolutePath());
			dest.getParentFile().mkdirs();

			FileOutputStream os = new FileOutputStream(dest);
			byte[] buf = new byte[1024];
			int len;
			while ((len = is.read(buf)) > 0) {
				os.write(buf, 0, len);
			}
			os.getFD().sync();
			os.close();
		} catch (Exception e) {
			Log.e(TAG, "Error: writeToFile failed,", e);
		}
	}


	private static final String TAG = "FileUtils";
}
