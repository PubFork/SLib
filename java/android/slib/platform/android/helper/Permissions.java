package slib.platform.android.helper;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Process;

public class Permissions {

	public static void requestPermissions(Activity activity, String[] permissions, int requestCode) {
		if (Build.VERSION.SDK_INT >= 23) {
			activity.requestPermissions(permissions, requestCode);
		}
	}

	public static void requestPermission(Activity activity, String permission, int requestCode) {
		requestPermissions(activity, new String[] {permission}, requestCode);
	}

	public static boolean checkPermission(Context context, String permission) {
		if (permission == null) {
			throw new IllegalArgumentException("permission is null");
		}
		return context.checkPermission(permission, Process.myPid(), Process.myUid()) == PackageManager.PERMISSION_GRANTED;
	}

	public static boolean grantPermission(Activity activity, String permission, int requestCode) {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			if (checkPermission(activity, permission)) {
				return true;
			} else {
				requestPermission(activity, permission, requestCode);
				return false;
			}
		} else {
			return true;
		}
	}

	public static boolean checkCameraPermission(Context context) {
		return checkPermission(context, Manifest.permission.CAMERA);
	}

	public static boolean grantCameraPermission(Activity activity, int requestCode) {
		return grantPermission(activity, Manifest.permission.CAMERA, requestCode);
	}

	public static boolean checkWriteExternalStoragePermission(Context context) {
		return checkPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE);
	}

	public static boolean grantWriteExternalStoragePermission(Activity activity, int requestCode) {
		return grantPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE, requestCode);
	}

}
